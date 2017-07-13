#include "watcher.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
Watcher::Watcher(int n):num_childs(n)
{
	wc_ebase=NULL;
	wc_exitEvent=NULL;
	wc_chldEvent=NULL;
}

Watcher::~Watcher()
{
	if(wc_ebase)
	{
		event_free(wc_exitEvent);
		event_free(wc_chldEvent);
		event_base_free(wc_ebase);
	}
	std::cout<<"Watcher Ended"<<std::endl;
}

bool Watcher::StartWatcher(const char *host,const char *serv)
{

	PluginList.push_back("plugin/plugin_static/plugin_static.so");
 //   PluginList.push_back("plugin/plugin_cgi/plugin_cgi.so");
	if(!worker.Init(this,host,serv))
	{
		std::cerr<<"Worker::Init() Fails"<<std::endl;
		return false;
	}
	if (num_childs > 0) {
		int child = 0;
		while (!child ) {
			if (num_childs > 0) {
				switch (fork()) {
				case -1:
					return -1;
				case 0:
					child = 1;
					break;
				default:
					num_childs--;
					break;
				}
			} else {
					wc_ebase = event_base_new();
					wc_exitEvent = evsignal_new(wc_ebase, SIGINT, WatcherExitSignal, wc_ebase);
					wc_chldEvent = evsignal_new(wc_ebase, SIGCHLD, WatcherChldSignal, this);
					evsignal_add(wc_exitEvent, NULL);//注册事件 
					evsignal_add(wc_chldEvent, NULL);
					event_base_dispatch(wc_ebase);
					return true;
			}
		}
	}
	worker.Run();
	return true;
}
void Watcher::WatcherExitSignal(evutil_socket_t signo,short event,void *arg)
{
	event_base_loopexit((struct event_base *)arg, NULL);//结束事件循环 
}

void Watcher::WatcherChldSignal(evutil_socket_t signo,short event,void *arg)
{
	Watcher *wc=static_cast<Watcher*>(arg);//强制转换回原来类型
	int status;
	pid_t pid;
	while((pid=waitpid(-1,&status,WNOHANG))>0)
	{
		++(wc->num_childs);
		std::cout<<"child" <<pid<<" terminated"<<std::endl;
	}
}
