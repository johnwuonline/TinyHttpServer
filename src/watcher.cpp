#include "watcher.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
/*
监控进程构造函数:
参数n:需要创建的worker的数量
*/
Watcher::Watcher(int n):num_childs(n)
{
	wc_ebase=NULL;
	wc_exitEvent=NULL;
	wc_chldEvent=NULL;
}
/*
监控进程折构函数:
释放libevent注册的事件和event_base
*/
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
/*
监控进程构造函数:
参数host:监听的IP
参数serv:监听的Port
*/
bool Watcher::StartWatcher(const char *host,const char *serv)
{

	PluginList.push_back("plugin/plugin_static/plugin_static.so");//动态库的路径
	if(!worker.Init(this,host,serv))
	{
		std::cerr<<"Worker::Init() Fails"<<std::endl;
		return false;
	}
	if (num_childs > 0) {
		int child = 0;//父进程标志，子进程则child=1.
		while (!child ) {//父进程
			if (num_childs > 0) {
				switch (fork()) {//fork子进程
				case -1:
					return -1;
				case 0:
					child = 1;//
					break;
				default:
					num_childs--;
					break;
				}
			} else {
					wc_ebase = event_base_new();
					wc_exitEvent = evsignal_new(wc_ebase, SIGINT, WatcherExitSignal, wc_ebase);//监控SIGINT
					wc_chldEvent = evsignal_new(wc_ebase, SIGCHLD, WatcherChldSignal, this);//监控SIGCHLD
					evsignal_add(wc_exitEvent, NULL);//注册事件 
					evsignal_add(wc_chldEvent, NULL);
					event_base_dispatch(wc_ebase);//事件循环阻塞
					return true;
			}
		}
	}
	worker.Run();//子进程处理连接请求
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
	while((pid=waitpid(-1,&status,WNOHANG))>0)//处理子进程终止
	{
		++(wc->num_childs);
		std::cout<<"child" <<pid<<" terminated"<<std::endl;
	}
}
