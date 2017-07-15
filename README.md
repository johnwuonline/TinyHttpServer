# TinyHttpServer


系统：centos 7.0

编写语言：C++

第三方库：libevent HttpParser

特性：
1. 采用watcher-worker模式，watcher负责监控子进程，worker负责客户端处理连接请求。

2.利用高性能Libevent网络库实现同步事件循环+非阻塞IO(Reactor模型)。

2.每个http连接维护一个有限状态机，根据连接的当前状态以及发生事件进行不同的处理。

3.使用连接池复用连接对象，避免频繁的new和delete。

4.支持插件开发

使用:

$ cd TinyHttpServer

$ make

$ cd plugin/plugin_static

$ make

$ cd ../..

$ ./TinyHttpServer

使用：

浏览器访问：http://127.0.0.1:9775/htdocs/index.htm

压力测试：

webbench -t 30 -c 10000 -2 --get http://127.0.0.1:9775/htdocs/index.html



