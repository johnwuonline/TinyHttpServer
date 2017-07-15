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
		static void WatcherExitSignal(evutil_socket_t signo,short event,void *arg);//����SIGINT�Ļص�����
		static void WatcherChldSignal(evutil_socket_t signo,short event,void *arg);//����SIGCHLD�Ļص�����
	public:
		Worker worker;
		typedef std::vector<std::string> plugin_list_t;
		plugin_list_t PluginList;//�������
	private:
		int num_childs;//worker(�ӽ���)����
		struct event_base *wc_ebase;//Reactor
		struct event *wc_exitEvent;//�ж��¼�
		struct event *wc_chldEvent;//�ӽ����¼�
		
};

#endif
