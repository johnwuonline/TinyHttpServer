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
		static void WatcherExitSignal(evutil_socket_t signo,short event,void *arg);//处理SIGINT的回调函数
		static void WatcherChldSignal(evutil_socket_t signo,short event,void *arg);//处理SIGCHLD的回调函数
	public:
		Worker worker;
		typedef std::vector<std::string> plugin_list_t;
		plugin_list_t PluginList;//插件链表
	private:
		int num_childs;//worker(子进程)个数
		struct event_base *wc_ebase;//Reactor
		struct event *wc_exitEvent;//中断事件
		struct event *wc_chldEvent;//子进程事件
		
};

#endif
