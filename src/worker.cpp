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
	UnloadPlugins();//卸载插件
	if(wk_exitEvent)
		event_free(wk_exitEvent);
	if(wk_ebase)
	{
			for(int i=0;i<con_pool_cur;++i)//释放连接池
		  {
				Connection *con=con_pool[i];
				delete con;
			}
			event_base_free(wk_ebase);
			std::cout<< "----total connection: " << wk_listener.listen_con_cnt << "----" << std::endl;
	}
	RemovePlugins();//移除插件
	std::cout<<"Worker Ended"<< std::endl;
}

void Worker::Run()
{
	std::cout<<"Worker Run"<< std::endl;
	wk_ebase=event_base_new();
	wk_listener.AddListenEvent();
	wk_exitEvent=evsignal_new(wk_ebase,SIGINT,WorkerExitSignal,wk_ebase);
	evsignal_add(wk_exitEvent,NULL);
	event_base_dispatch(wk_ebase);//事件循环
	return; 
}

/*监控进程初始化*/
bool Worker::Init(Watcher *wc,const char *host,const char *serv)
{
	wk_wc=wc;
	InitConPool();//初始化连接池
	if (!wk_listener.InitListener(this,host,serv))//建立监听套接字
	{
		std::cerr<<"Worker: Listener::InitListener()"<< std::endl;
		return false;
	}
	if (!(SetupPlugins() && LoadPlugins()))//动态库加载插件
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
/*设置插件*/
bool Worker::SetupPlugins()
{
	std::string path;
	
	for(int i=0;i<wk_wc->PluginList.size();++i)
	{
		path=wk_wc->PluginList[i];
		
		void *so=dlopen(path.c_str(),RTLD_LAZY);//加载指定共享库文件，得到映射句柄
		if(!so)
		{
			std::cerr << dlerror() << std::endl;
			return false;
		}
		Plugin::SetupPlugin setup_plugin = (Plugin::SetupPlugin)dlsym(so, "SetupPlugin");//返回加载库内指定符号的地址指针
		Plugin::RemovePlugin remove_plugin = (Plugin::RemovePlugin)dlsym(so, "RemovePlugin");
		if (!setup_plugin || !remove_plugin)
		{
			std::cerr << dlerror() << std::endl;
			dlclose(so);
			return false;
		}
		Plugin *plugin = setup_plugin();//得到插件 
		if (!plugin)
		{
			dlclose(so);
			return false;
		}

		plugin->setup_plugin = setup_plugin;//设置插件参数 
		plugin->remove_plugin = remove_plugin;
		plugin->plugin_so = so;
		plugin->plugin_index = i;
		//为插件分配空间
		Plugin* *temp=static_cast<Plugin* *>(realloc(w_plugins, sizeof(*w_plugins)*(w_plugin_cnt+1)));
		if(temp!=NULL)
		{
			w_plugins=temp;
			w_plugins[w_plugin_cnt++] = plugin;//添加插件
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

/*加载插件*/
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
		remove_plugin(plugin);//释放插件实例
		dlclose(so);//释放共享库
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
/*初始化连接池:使用vector<Connection*>实现
连接池大小:200
*/
void Worker::InitConPool()
{
	con_pool_size=200;
	con_pool_cur=0;
	con_pool.resize(con_pool_size);
}
/*取空闲的一个连接*/
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
	else//连接池已满，重新分配空间
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
/*关闭连接*/
void Worker::CloseCon(Connection *con)
{
	std::cout<<"Worker::CloseCon"<<std::endl;
	Worker *worker = con->con_worker;
	if(con->con_read_event&&con->con_write_event)
	{
		ConnectionMap::iterator con_iter=worker->wk_con_map.find(con->con_sockfd);
		worker->wk_con_map.erase(con_iter);
	}
	con->ResetCon();//清空连接数据
	if(!worker->AddConToFreePool(con))//将连接释放到连接池
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


