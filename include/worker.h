#ifndef WORKER_H
#define WORKER_H
#include "listener.h"

#include <string>
#include <map>
#include <vector>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"
class Connection;
class Listener;
class Watcher;
class Plugin;
class Worker
{
	public:
		Worker();
		~Worker();
		void Run();
		bool Init(Watcher *wc,const char* host,const char* serv);
		static void WorkerExitSignal(evutil_socket_t signo,short event,void *arg);
		
		Connection* NewCon();
		static void CloseCon(Connection *con);
	public:
		Watcher *wk_wc;
		struct event_base *wk_ebase;
		typedef std::map<evutil_socket_t, Connection *> ConnectionMap;
		ConnectionMap wk_con_map;
		
		Plugin*			*w_plugins;
		int			 w_plugin_cnt;
		
	private:
		void InitConPool();
		Connection* GetFreeCon();
		bool AddConToFreePool(Connection *con);
		static void FreeCon(Connection *con);
		
		bool SetupPlugins();		
		bool LoadPlugins(); 
		void RemovePlugins();
		void UnloadPlugins();
	private:
		struct event *wk_exitEvent;
		Listener wk_listener;
		typedef std::vector<Connection*> con_pool_t;
		con_pool_t con_pool;
		int con_pool_size;
		int con_pool_cur;
};

#endif
