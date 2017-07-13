#include "plugin.h"
#include "connection.h"
#include "worker.h"
#include "http.h"

#include "event2/event.h"
#include "event2/util.h"

#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>

#include <iostream>
#include <string>

typedef enum
{
	INIT,
	READ,
	DONE,
	NOT_EXIST,
	NOT_ACCESS
}static_state_t;//状态值枚举
typedef struct StaticData
{
	StaticData():s_fd(-1),s_state(INIT){}//默认构造函数
	int s_fd;
	std::string s_buf;
	std::string s_data;
	static_state_t s_state; 
}static_data_t; 

class PluginStatic:public Plugin//继承Plugin基类 
{
	virtual bool Init(Connection *con,int index)//重写基类Init 
	{
		static_data_t *data=new static_data_t();
		data->s_buf.reserve(10*1024);
		con->plugin_data_slots[index]=data;
		return true;
	}
	virtual bool ResponseStart(Connection *con,int index)
	{
		static_data_t  *data = static_cast<static_data_t*>(con->plugin_data_slots[index]);
		HttpRequest *request=con->http_req_parsed;//已解析的请求 
		
		regex_t reg;
		regmatch_t pmatch;
		int ret;
		//int regcomp (regex_t *compiled, const char *pattern, int cflags)  
		//把指定的正则表达式pattern编译成一种特定的数据格式compiled
		regcomp(&reg, "^/htdocs/[^/]*$", REG_EXTENDED);
		//int regexec (regex_t *compiled, char *string, size_t nmatch, regmatch_t matchptr [], int eflags) 
		//当我们编译好正则表达式后，就可以用regexec 匹配我们的目标文本串了，
		//如果在编译正则表达式的时候没有指定cflags的参数为REG_NEWLINE，
		//则默认情况下是忽略换行符的，也就是把整个文本串当作一个字符串处理。执行成功返回０。  
        ret = regexec(&reg, request->http_url.c_str(), 1, &pmatch, 0);
        
        if(ret)
        {
			data->s_state=NOT_ACCESS;
		}
		else
		{
			std::string path=request->http_url.substr(1);
			if(access(path.c_str(),R_OK)==-1)
			{
				data->s_state=NOT_EXIST;
			}
			else
			{
				data->s_state=INIT;
			}
		}
		return true;
	}
	virtual plugin_state_t Write(Connection *con,int index)
	{
		static_data_t  *data = static_cast<static_data_t*>(con->plugin_data_slots[index]);
        HttpRequest *request = con->http_req_parsed;
        
        if(data->s_state==INIT)
        {
			data->s_state=READ;
			data->s_fd=open(request->http_url.substr(1).c_str(),O_RDONLY);
		}
		else if(data->s_state==NOT_ACCESS)
		{
			con->http_response.http_code    = 404;
            con->http_response.http_phrase 	= "Access Deny";
            return PLUGIN_READY;
		}
		else if (data->s_state == NOT_EXIST)
        {
            con->http_response.http_code    = 403;
            con->http_response.http_phrase 	= "File don't exist";
            return PLUGIN_READY;
        }
		int ret = read(data->s_fd, &data->s_buf[0], data->s_buf.capacity());

        if (ret <= 0)
        {
            data->s_state = DONE;
            con->http_response.http_body += data->s_data;
            return PLUGIN_READY;
        }
        else
        {
            data->s_data.append(&data->s_buf[0], 0, ret);
            return PLUGIN_NOT_READY;
        }
	}
	virtual bool ResponseEnd(Connection *con, int index)
    {
        static_data_t *data = static_cast<static_data_t*>(con->plugin_data_slots[index]);

        if (data->s_state == DONE)
        {
            close(data->s_fd);
            data->s_fd = -1;
            data->s_data.clear();
        }
        
        return true;
    }
    virtual void Close(Connection *con, int index)
    {
        static_data_t *data = static_cast<static_data_t*>(con->plugin_data_slots[index]);

        if (data->s_fd != -1)
        {
            close(data->s_fd);
        }

        delete data;
    }
};
extern "C" Plugin* SetupPlugin()
{
    return new PluginStatic();
}
extern "C" Plugin* RemovePlugin(Plugin *plugin)
{
    delete plugin;
}




