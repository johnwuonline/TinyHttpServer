#ifndef LISTENER_H
#define LISTENER_H
#include <string>

#include "event2/event.h"
#include "event2/util.h"

#include "util.h"

class Worker;
class Listener
{
	public:
		Listener();
		~Listener();
		
		bool InitListener(Worker *worker,const char* host,const char *serv);
		void AddListenEvent();
		static void ListenEventCb(evutil_socket_t fd,short event,void *arg); 
	public:
		Worker *listen_worker;
		evutil_socket_t listen_sockfd;
		struct sockaddr_in listen_addr;
		struct event *listen_event;
		int listen_con_cnt;
		
};

#endif
