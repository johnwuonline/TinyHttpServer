#ifndef CONNECTION_H
#define CONNECTION_H
#include <string>
#include <queue>
#include "event2/event.h"
#include "event2/util.h"
#include "http.h"
#include "plugin.h"
typedef enum
{
	CON_STATE_CONNECT,
	CON_STATE_REQUEST_START,
	CON_STATE_READ,
	CON_STATE_REQUEST_END,
	CON_STATE_HANDLE_REQUEST,
	CON_STATE_RESPONSE_START,
	CON_STATE_WRITE,
	CON_STATE_RESPONSE_END,
	CON_STATE_ERROR
} connection_state_t;

typedef enum
{
    REQ_ERROR,
    REQ_IS_COMPLETE,
    REQ_NOT_COMPLETE
} request_state_t;


class Worker;
class Connection
{
	public:
		Connection();
		~Connection();
		
		bool InitConnection(Worker *worker);
		void ResetCon();
		static void ConEventCb(evutil_socket_t fd,short event, void *arg);
	
	public:
		
		struct event *con_read_event;
		struct event *con_write_event;
		Worker *con_worker;
		evutil_socket_t con_sockfd;

		//HTTP请求相关
		typedef std::queue<HttpRequest*> req_queue_t;
		req_queue_t	req_queue;//请求队列 
		HttpRequest	*http_req_parser; 
		HttpRequest	*http_req_parsed;//已解析请求  
		HttpResponse http_response;

		//插件数据结构
		void* *plugin_data_slots;
		int	plugin_cnt;
		int	plugin_next;
	private:
		//TCP接受发送缓冲区
		std::string con_intmp;
		std::string con_inbuf;
		std::string con_outbuf;
		
		connection_state_t con_state;
		request_state_t	req_state;
		bool con_want_write;
		bool con_want_read;
		int con_req_cnt;
		HttpParser http_parser;
	private:
		//读写事件相关
		void WantRead();
		void NotWantRead();
		void WantWrite();
		void NotWantWrite();
		void SetWriteEvent();
		void UnsetWriteEvent();
		
		void ResetConnection();

		//状态机
		void SetState(connection_state_t state);
		bool StateMachine();
		//解析HTTP请求与
		request_state_t GetParsedRequest();
		void SetErrorResponse();

		//插件处理请求相关函数
		bool InitPluginDataSlots();
		void FreePluginDataSlots();

		bool PluginRequestStart();
		bool PluginRead();
		bool PluginRequestEnd();
		bool PluginResponseStart();
		plugin_state_t PluginWrite();
		bool PluginResponseEnd();
};

#endif
