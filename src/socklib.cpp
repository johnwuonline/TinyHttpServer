#include "socklib.h"

/*
函数功能:创建一个TCP套接字，给它捆绑相应的端口，并允许接受外来的连接请求
*/
int tcp_listen(const char *host,const char *serv,socklen_t *addrlenp)
{
	int listenfd,n;
	const int on=1;
	struct addrinfo hints,*res,*ressave;
	
	bzero(&hints,sizeof(struct addrinfo));//地址置0
	hints.ai_flags=AI_PASSIVE;//套接字被动打开
	hints.ai_family=AF_UNSPEC;//函数返回适用于指定主机名和服务名且适合任何协议族的地址
	hints.ai_socktype=SOCK_STREAM;
	
	if((n=getaddrinfo(host,serv,&hints,&res))!=0)//通过res返回一个指向addrinfo结构链表的指针，成功返回0.
			return -1;
	ressave=res;
	
	do{
		if((listenfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))<0)//得到监听套接字
				continue;
		evutil_make_socket_nonblocking(listenfd);//非阻塞IO
		setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));//重用地址，解决“Address already use”，TimeWait。
		if(bind(listenfd,res->ai_addr,res->ai_addrlen)==0)//绑定本地协议地址到套接字
			break;//成功则跳出循环
		close(listenfd);//不成功，关闭套接字继续下一次循环
	}while((res=res->ai_next)!=NULL);
	
	if(res==NULL)
			return -1;
	if(listen(listenfd,LISTENQ)!=0)
		return -1;
	if(addrlenp)
		*addrlenp=res->ai_addrlen;//得到地址长度
	
	freeaddrinfo(ressave);
	
	return listenfd;
}

