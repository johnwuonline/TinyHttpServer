#ifndef PLUGIN_H
#define PLUGIN_H
class Worker;
class Connection;
typedef enum
{
	PLUGIN_READY,
	PLUGIN_NOT_READY,
	PLUGIN_ERROR
}plugin_state_t;
class Plugin
{
	public:
		Plugin();
		virtual ~Plugin();
		
		virtual bool Init(Connection *con,int index);
		virtual bool RequestStart(Connection *con,int index);
		virtual bool Read(Connection *con,int index);
		virtual bool RequestEnd(Connection *con,int index);
		virtual bool ResponseStart(Connection *con, int index);
		virtual plugin_state_t Write(Connection *con, int index);
		virtual bool ResponseEnd(Connection *con, int index);
		virtual void Close(Connection *con, int index);
		virtual bool Trigger(Worker* worker, int index);
		virtual bool LoadPlugin(Worker* worker, int index);
		virtual void FreePlugin(Worker* worker, int index);
		
		typedef Plugin* (*SetupPlugin)();
		typedef void	(*RemovePlugin)(Plugin *plugin);
		
		SetupPlugin setup_plugin;//函数指针，参数为空，返回值为Plugin*
		RemovePlugin remove_plugin;//函数指针，参数为Plugin *,返回值为空

		void* plugin_data;
		void* plugin_so;
		int   plugin_index;
		bool  plugin_is_loaded;
};
extern const char * plugin_list[];
#endif
