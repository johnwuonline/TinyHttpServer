#ifndef SOCKLIB_H
#define SOCKLIB_H
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "event2/event.h"
#include "event2/util.h"
#include <strings.h>
int tcp_listen(const char *host,const char *serv,socklen_t *addrlenp);
#define LISTENQ 5
#endif
