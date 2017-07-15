#include "watcher.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
/*
��ؽ��̹��캯��:
����n:��Ҫ������worker������
*/
Watcher::Watcher(int n):num_childs(n)
{
	wc_ebase=NULL;
	wc_exitEvent=NULL;
	wc_chldEvent=NULL;
}
/*
��ؽ����۹�����:
�ͷ�libeventע����¼���event_base
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
��ؽ��̹��캯��:
����host:������IP
����serv:������Port
*/
bool Watcher::StartWatcher(const char *host,const char *serv)
{

	PluginList.push_back("plugin/plugin_static/plugin_static.so");//��̬���·��
	if(!worker.Init(this,host,serv))
	{
		std::cerr<<"Worker::Init() Fails"<<std::endl;
		return false;
	}
	if (num_childs > 0) {
		int child = 0;//�����̱�־���ӽ�����child=1.
		while (!child ) {//������
			if (num_childs > 0) {
				switch (fork()) {//fork�ӽ���
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
					wc_exitEvent = evsignal_new(wc_ebase, SIGINT, WatcherExitSignal, wc_ebase);//���SIGINT
					wc_chldEvent = evsignal_new(wc_ebase, SIGCHLD, WatcherChldSignal, this);//���SIGCHLD
					evsignal_add(wc_exitEvent, NULL);//ע���¼� 
					evsignal_add(wc_chldEvent, NULL);
					event_base_dispatch(wc_ebase);//�¼�ѭ������
					return true;
			}
		}
	}
	worker.Run();//�ӽ��̴�����������
	return true;
}
void Watcher::WatcherExitSignal(evutil_socket_t signo,short event,void *arg)
{
	event_base_loopexit((struct event_base *)arg, NULL);//�����¼�ѭ�� 
}

void Watcher::WatcherChldSignal(evutil_socket_t signo,short event,void *arg)
{
	Watcher *wc=static_cast<Watcher*>(arg);//ǿ��ת����ԭ������
	int status;
	pid_t pid;
	while((pid=waitpid(-1,&status,WNOHANG))>0)//�����ӽ�����ֹ
	{
		++(wc->num_childs);
		std::cout<<"child" <<pid<<" terminated"<<std::endl;
	}
}
