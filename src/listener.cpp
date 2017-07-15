#include "listener.h"
#include "worker.h"
#include "connection.h"
#include "watcher.h"
#include "socklib.h"
#include <iostream>
Listener::Listener()//构造函数
{
	listen_event=NULL;
	listen_con_cnt=0;
	std::cout<<"Init Listener"<<std::endl;
}

Listener::~Listener()//折构函数
{
	if(listen_event)
	{
		event_free(listen_event);
	}
	close(listen_sockfd);
	std::cout<<"Listener Ended"<<std::endl;
}

bool Listener::InitListener(Worker *worker,const char *host,const char *serv)
{
	if((listen_sockfd=tcp_listen(host,serv,NULL))<0)//建立监听套接字
	{
		std::cerr<<"Listen Failed"<<std::endl;
		return false;
	}
	listen_worker=worker;
	return true;
}

void Listener::AddListenEvent()
{
	std::cout<<"Listener::AddListenEvent"<<std::endl;
	listen_event=event_new(listen_worker->wk_ebase,listen_sockfd,EV_READ|EV_PERSIST,ListenEventCb,this);//监听连接读事件
	event_add(listen_event,NULL);
}

void Listener::ListenEventCb(evutil_socket_t sockfd, short event, void *arg)
{
	std::cout<<"Listener::ListenEventCb"<<std::endl;
	evutil_socket_t con_fd;
	struct sockaddr_in con_addr;
	socklen_t addr_len=sizeof(con_addr);

	if(-1==(con_fd=accept(sockfd,(struct sockaddr*)&con_addr,&addr_len))) //建立连接fd
	{
		std::cerr<<"accept error"<<std::endl;
		return;
	}
	Listener *listener=static_cast<Listener*>(arg);
	Connection *con=listener->listen_worker->NewCon();//得到连接池的一个连接
	if (con == NULL)
	{
		std::cerr<< "Listener::ListenEventCallback(): NewCon()"<<std::endl;
		return ;
	}
	con->con_sockfd = con_fd;//传递连接文件描述符
	std::cout << "listen accept: " << con->con_sockfd << " by process " << getpid() <<std::endl;
	if(!con->InitConnection(listener->listen_worker))//初始化连接，分配空间，建立事件监听
	{
		std::cerr<<"InitConnection error"<<std::endl;
		Worker::CloseCon(con);
		return;
	}
	con->con_worker->wk_con_map[con->con_sockfd]=con;
	++(listener->listen_con_cnt);
}


