#include "worker.h"
#include "connection.h"
#include "watcher.h"
#include "plugin.h"

#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>
Worker::Worker()
{
	wk_ebase=NULL;
	wk_exitEvent=NULL;
	wk_wc=NULL;
	w_plugins	= NULL;
	w_plugin_cnt	= 0;
}

Worker::~Worker()
{
	UnloadPlugins();//ж�ز��
	if(wk_exitEvent)
		event_free(wk_exitEvent);
	if(wk_ebase)
	{
			for(int i=0;i<con_pool_cur;++i)//�ͷ����ӳ�
		  {
				Connection *con=con_pool[i];
				delete con;
			}
			event_base_free(wk_ebase);
			std::cout<< "----total connection: " << wk_listener.listen_con_cnt << "----" << std::endl;
	}
	RemovePlugins();//�Ƴ����
	std::cout<<"Worker Ended"<< std::endl;
}

void Worker::Run()
{
	std::cout<<"Worker Run"<< std::endl;
	wk_ebase=event_base_new();
	wk_listener.AddListenEvent();
	wk_exitEvent=evsignal_new(wk_ebase,SIGINT,WorkerExitSignal,wk_ebase);
	evsignal_add(wk_exitEvent,NULL);
	event_base_dispatch(wk_ebase);//�¼�ѭ��
	return; 
}

/*��ؽ��̳�ʼ��*/
bool Worker::Init(Watcher *wc,const char *host,const char *serv)
{
	wk_wc=wc;
	InitConPool();//��ʼ�����ӳ�
	if (!wk_listener.InitListener(this,host,serv))//���������׽���
	{
		std::cerr<<"Worker: Listener::InitListener()"<< std::endl;
		return false;
	}
	if (!(SetupPlugins() && LoadPlugins()))//��̬����ز��
	{
		std::cerr<< "Worker: SetupPlugins() && LoadPlugins()" << std::endl;
		return false;
	}
	return true;
}

void Worker::WorkerExitSignal(evutil_socket_t signo,short event,void *arg)
{
	event_base_loopexit((struct event_base*)arg,NULL);
}
/*���ò��*/
bool Worker::SetupPlugins()
{
	std::string path;
	
	for(int i=0;i<wk_wc->PluginList.size();++i)
	{
		path=wk_wc->PluginList[i];
		
		void *so=dlopen(path.c_str(),RTLD_LAZY);//����ָ��������ļ����õ�ӳ����
		if(!so)
		{
			std::cerr << dlerror() << std::endl;
			return false;
		}
		Plugin::SetupPlugin setup_plugin = (Plugin::SetupPlugin)dlsym(so, "SetupPlugin");//���ؼ��ؿ���ָ�����ŵĵ�ַָ��
		Plugin::RemovePlugin remove_plugin = (Plugin::RemovePlugin)dlsym(so, "RemovePlugin");
		if (!setup_plugin || !remove_plugin)
		{
			std::cerr << dlerror() << std::endl;
			dlclose(so);
			return false;
		}
		Plugin *plugin = setup_plugin();//�õ���� 
		if (!plugin)
		{
			dlclose(so);
			return false;
		}

		plugin->setup_plugin = setup_plugin;//���ò������ 
		plugin->remove_plugin = remove_plugin;
		plugin->plugin_so = so;
		plugin->plugin_index = i;
		//Ϊ�������ռ�
		Plugin* *temp=static_cast<Plugin* *>(realloc(w_plugins, sizeof(*w_plugins)*(w_plugin_cnt+1)));
		if(temp!=NULL)
		{
			w_plugins=temp;
			w_plugins[w_plugin_cnt++] = plugin;//��Ӳ��
		}
		else
		{
			free(w_plugins);
			std::cerr << "Error rellocating memory" << std::endl;
			return false;
		}

	}
	return true;
}

/*���ز��*/
bool Worker::LoadPlugins()
{
	Plugin *plugin;
	for(int i=0;i<w_plugin_cnt;++i)
	{
		plugin=w_plugins[i];
		if(plugin->LoadPlugin(this,i))
		{
			plugin->plugin_is_loaded=true;
		}
		else
		{
			std::cerr<< "Worker: Plugin::LoadPlugin()" << std::endl;
			return false;
		}
	}
	return true;
} 

void Worker::RemovePlugins()
{
	Plugin *plugin;

	for (int i = 0; i < w_plugin_cnt; ++i)
	{
		plugin = w_plugins[i];
		Plugin::RemovePlugin remove_plugin = plugin->remove_plugin;
		void *so = plugin->plugin_so;
		remove_plugin(plugin);//�ͷŲ��ʵ��
		dlclose(so);//�ͷŹ����
	}
	free(w_plugins);
}

void Worker::UnloadPlugins()
{
	Plugin *plugin;

	for (int i = 0; i < w_plugin_cnt; ++i)
	{
		plugin = w_plugins[i];
		if (plugin->plugin_is_loaded)
		{
			plugin->FreePlugin(this, i);
		}
	}
}
/*��ʼ�����ӳ�:ʹ��vector<Connection*>ʵ��
���ӳش�С:200
*/
void Worker::InitConPool()
{
	con_pool_size=200;
	con_pool_cur=0;
	con_pool.resize(con_pool_size);
}
/*ȡ���е�һ������*/
Connection* Worker::GetFreeCon()
{
	Connection *con=NULL;
	if(con_pool_cur>0)
	{
		con=con_pool.at(--con_pool_cur);
	}
	return con;
}

bool Worker::AddConToFreePool(Connection *con)
{
	bool res=false;
	if(con_pool_cur<con_pool_size)
	{
		con_pool.at(con_pool_cur++)=con;
		res=true;
	}
	else//���ӳ����������·���ռ�
	{
		size_t newsize=con_pool_size*2;
		con_pool.resize(newsize);
		con_pool_size=newsize;
		con_pool.at(con_pool_cur++)=con;
		res=true;
	}
	return res;
}

void Worker::FreeCon(Connection *con)
{
	delete con;
}
/*�ر�����*/
void Worker::CloseCon(Connection *con)
{
	std::cout<<"Worker::CloseCon"<<std::endl;
	Worker *worker = con->con_worker;
	if(con->con_read_event&&con->con_write_event)
	{
		ConnectionMap::iterator con_iter=worker->wk_con_map.find(con->con_sockfd);
		worker->wk_con_map.erase(con_iter);
	}
	con->ResetCon();//�����������
	if(!worker->AddConToFreePool(con))//�������ͷŵ����ӳ�
		FreeCon(con);
	return;
}

Connection* Worker::NewCon()
{
	Connection *con=GetFreeCon();
	if(NULL==con)
	{
		con=new Connection();
		if(NULL==con)
		{
			return NULL;
		}
	}
	return con;
}


