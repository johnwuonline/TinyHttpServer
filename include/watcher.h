#ifndef WATCHER_H
#define WATCHER_H
#include "worker.h"

#include "event2/event.h"
#include "event2/util.h"
class Watcher
{
	public:
		Watcher(int n);
		~Watcher();
		
		bool StartWatcher(const char *host,const char *serv);
		static void WatcherExitSignal(evutil_socket_t signo,short event,void *arg);//处理SIGINT
		static void WatcherChldSignal(evutil_socket_t signo,short event,void *arg);//处理SIGCHLD
	public:
		Worker worker;
		typedef std::vector<std::string> plugin_list_t;
		plugin_list_t PluginList;
	private:
		int num_childs;
		struct event_base *wc_ebase;
		struct event *wc_exitEvent;
		struct event *wc_chldEvent;
		
};

#endif
