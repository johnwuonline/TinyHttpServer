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
/*������࣬��̬���д����̳л���ӿ�*/
class Plugin
{
	public:
		Plugin();
		virtual ~Plugin();

		//����HTTP������Ӧ���
		virtual bool Init(Connection *con,int index);
		virtual bool RequestStart(Connection *con,int index);
		virtual bool Read(Connection *con,int index);
		virtual bool RequestEnd(Connection *con,int index);
		virtual bool ResponseStart(Connection *con, int index);
		virtual plugin_state_t Write(Connection *con, int index);
		virtual bool ResponseEnd(Connection *con, int index);
		virtual void Close(Connection *con, int index);
		virtual bool LoadPlugin(Worker* worker, int index);
		virtual void FreePlugin(Worker* worker, int index);
		
		typedef Plugin* (*SetupPlugin)();
		typedef void	(*RemovePlugin)(Plugin *plugin);
		
		SetupPlugin setup_plugin;//����ָ�룬����Ϊ�գ�����ֵΪPlugin*
		RemovePlugin remove_plugin;//����ָ�룬����ΪPlugin *,����ֵΪ��

		
		void* plugin_data;//�������
		void* plugin_so;//���λ��
		int   plugin_index;//�������
		bool  plugin_is_loaded;//������ر�־
};
extern const char * plugin_list[];
#endif
