# TinyHttpServer


ϵͳ��centos 7.0
��д���ԣ�C++
�������⣺libevent HttpParser

���ԣ�
1. ����watcher-workerģʽ��watcher�������ӽ��̣�worker����ͻ��˴�����������
2.���ø�����Libevent�����ʵ��ͬ���¼�ѭ��+������IO(Reactorģ��)��
2.ÿ��http����ά��һ������״̬�����������ӵĵ�ǰ״̬�Լ������¼����в�ͬ�Ĵ���
3.ʹ�����ӳظ������Ӷ��󣬱���Ƶ����new��delete��
4.֧�ֲ������

ʹ��:

$ cd TinyHttpServer
$ make
$ cd plugin/plugin_static
$ make
$ cd ../..
$ ./TinyHttpServer

ѹ�����ԣ�

webbench -t 30 -c 10000 -2 --get http://127.0.0.1:9775/htdocs/index.html



