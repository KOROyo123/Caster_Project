#include "MsgSwitch.h"

#include <string>

#include <event2/event.h>
#include <event2/thread.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

typedef void (*datacallback)(const char *szdata);

class redis_client
{
private:
    event_base *_base;

    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_client();
    ~redis_client();

    int Init(const char *ip, int port, const char *pwd);

    int start();
    int stop();

    int add_pub(const char *channel, const char *data);

    int add_sub(const char *channel, int (*pFunc)(const char *szdata));

    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

    static void Redis_Recv_Callback(redisAsyncContext *c, void *r, void *privdata);

    static void *event_base_thread(void *arg);
};

redis_client redismq;

redis_client::redis_client()
{
    _base = event_base_new();
}

redis_client::~redis_client()
{
    event_base_free(_base);
}

int redis_client::Init(const char *ip, int port, const char *pwd)
{
    _redis_IP = ip;
    _redis_port = port;
    _redis_Requirepass = pwd;

    // 初始化redis连接
    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, _redis_IP.c_str(), _redis_port);
    struct timeval tv = {0};
    tv.tv_sec = 10;
    options.connect_timeout = &tv;

    _pub_context = redisAsyncConnectWithOptions(&options);
    if (_pub_context->err)
    {
        /* Let *c leak for now... */
        // spdlog::error("redis eror: {}", _pub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_pub_context, _base);
    redisAsyncSetConnectCallback(_pub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_pub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_pub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    _sub_context = redisAsyncConnectWithOptions(&options);
    if (_sub_context->err)
    {
        /* Let *c leak for now... */
        // spdlog::error("redis eror: {}", _sub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_sub_context, _base);
    redisAsyncSetConnectCallback(_sub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_sub_context, Redis_Disconnect_Cb);

    redisAsyncCommand(_sub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    return 0;
}

int redis_client::add_pub(const char *channel, const char *data)
{

    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH %s %s", channel, data);

    return 0;
}

int redis_client::add_sub(const char *channel, int (*pFunc)(const char *szdata))
{

    redisAsyncCommand(_sub_context, Redis_Recv_Callback, (void *)pFunc, "SUBSCRIBE %s", channel);

    return 0;
}

void redis_client::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
}

void redis_client::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
}

void redis_client::Redis_Recv_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto func = (datacallback)privdata;

    // evbuffer *evbuf = bufferevent_get_output(svr->_bev);

    if (reply->element[2]->type == REDIS_REPLY_INTEGER)
    {
    }
    else
    {
        func(reply->element[2]->str);
    }
}

void *redis_client::event_base_thread(void *arg)
{

    event_base *base = (event_base *)arg;

    evthread_make_base_notifiable(base);

    // event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

    event_base_dispatch(base);

    return nullptr;
}

int redis_client::start()
{
#ifdef WIN32
    evthread_use_windows_threads(); // libevent启用windows线程函数
#else
    evthread_use_pthreads(); // libenvet启用linux线程函数
#endif

    pthread_t id;
    int ret = pthread_create(&id, NULL, event_base_thread, _base);
    if (ret)
    {
        return 1;
    }
    return 0;

    return 0;
}

int redis_client::stop()
{

    event_base_loopbreak(_base);

    return 0;
}

int Init(const char *groupid, const char *szIP, int port)
{
    redismq.Init(szIP, port, "snLW_1234");

    return 0;
}

int PostMsg(const char *moduleId, const char *dataId, const char *szdata)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq.add_pub(channel.c_str(), szdata);

    return 0;
}

int PostMsg1(const char *moduleId, int dataId, const char *szdata)
{

    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq.add_pub(channel.c_str(), szdata);
    return 0;
}

int RegisterNotify(const char *moduleId, const char *dataId, int (*pFunc)(const char *szdata))
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq.add_sub(channel.c_str(), pFunc);

    return 0;
}

int RegisterNotify1(const char *moduleId, int dataId, int (*pFunc)(const char *szdata))
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq.add_sub(channel.c_str(), pFunc);

    return 0;
}

int ShutDown()
{
    redismq.stop();

    return 0;
}

int StartMQ()
{
    redismq.start();
    return 0;
}
