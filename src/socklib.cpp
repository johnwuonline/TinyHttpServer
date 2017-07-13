#include "socklib.h"
int tcp_listen(const char *host,const char *serv,socklen_t *addrlenp)
{
	int listenfd,n;
	const int on=1;
	struct addrinfo hints,*res,*ressave;
	
	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_flags=AI_PASSIVE;
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	
	if((n=getaddrinfo(host,serv,&hints,&res))!=0)
			return -1;
	ressave=res;
	
	do{
		if((listenfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))<0)
				continue;
		evutil_make_socket_nonblocking(listenfd);
		setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));//重用地址，解决“Address already use”.
		if(bind(listenfd,res->ai_addr,res->ai_addrlen)==0)
			break;
		close(listenfd);
	}while((res=res->ai_next)!=NULL);
	
	if(res==NULL)
			return -1;
	if(listen(listenfd,LISTENQ)!=0)
		return -1;
	if(addrlenp)
		*addrlenp=res->ai_addrlen;
	
	freeaddrinfo(ressave);
	
	return listenfd;
}

