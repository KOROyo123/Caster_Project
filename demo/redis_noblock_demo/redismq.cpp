#include "redismq.h"
#include "string.h"

redis_mq::redis_mq()
{
}

redis_mq::~redis_mq()
{
}

int redis_mq::init(const char *ip, int port, const char *pwd)
{
#ifdef WIN32
    evthread_use_windows_threads(); // libevent启用windows线程函数
#else
    evthread_use_pthreads(); // libenvet启用linux线程函数
#endif

    _redis_IP = ip;
    _redis_port = port;
    _redis_Requirepass = pwd;

    _pub_base = event_base_new();
    _sub_base = event_base_new();

    _pub_con_check_tv.tv_sec = 5;
    _pub_con_check_tv.tv_usec = 0;
    _pub_con_check_ev = event_new(_pub_base, -1, EV_PERSIST, Pub_Check_Event, this);
    event_add(_pub_con_check_ev, &_pub_con_check_tv);

    _sub_con_check_tv.tv_sec = 5;
    _sub_con_check_tv.tv_usec = 0;
    _sub_con_check_ev = event_new(_sub_base, -1, EV_PERSIST, Sub_Check_Event, this);
    event_add(_sub_con_check_ev, &_sub_con_check_tv);

    _reconnect_pub_ev = event_new(_pub_base, -1, EV_PERSIST, Reconnect_Pub_Event, this);
    _reconnect_sub_ev = event_new(_sub_base, -1, EV_PERSIST, Reconnect_Sub_Event, this);

    _pub_data_ev = event_new(_pub_base, -1, EV_PERSIST, Pub_to_redis_Event, this);
    _set_value_ev = event_new(_pub_base, -1, EV_PERSIST, Set_to_redis_Event, this);
    _sub_channel_ev = event_new(_sub_base, -1, EV_PERSIST, Sub_to_redis_Event, this);
    _cal_channel_ev = event_new(_sub_base, -1, EV_PERSIST, Cal_to_redis_Event, this);

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
        return 0;
    }

    redisLibeventAttach(_pub_context, _pub_base);
    redisAsyncSetConnectCallback(_pub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_pub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_pub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    _sub_context = redisAsyncConnectWithOptions(&options);
    if (_sub_context->err)
    {
        /* Let *c leak for now... */
        // spdlog::error("redis eror: {}", _sub_context->errstr);
        return 0;
    }

    redisLibeventAttach(_sub_context, _sub_base);
    redisAsyncSetConnectCallback(_sub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_sub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_sub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    return 0;
}

int redis_mq::start()
{
    pthread_t id1, id2;
    int ret1 = pthread_create(&id1, NULL, libevent_thread, _pub_base);
    int ret2 = pthread_create(&id2, NULL, libevent_thread, _sub_base);

    return 0;
}

int redis_mq::stop()
{
    event_base_loopbreak(_pub_base);
    event_base_loopbreak(_sub_base);
    return 0;
}

int redis_mq::add_pub_queue(const char *channel, const char *data)
{
    if (_PUB_QUEUE.size() > QUEUE_LIMIT)
    {
        return 1;
    }
    Pub_Data_Item *item = new Pub_Data_Item(channel, data);
    _PUB_QUEUE.push(item);
    event_active(_pub_data_ev, 0, 0);
    return 0;
}

int redis_mq::add_set_queue(const char *channel, const char *data)
{
    if (_SET_QUEUE.size() > QUEUE_LIMIT)
    {
        return 1;
    }
    Set_Value_Item *item = new Set_Value_Item(channel, data);
    _SET_QUEUE.push(item);
    event_active(_set_value_ev, 0, 0);
    return 0;
    return 0;
}

int redis_mq::add_sub_queue(const char *channel, int (*pFunc)(const char *szdata))
{
    if (_SUB_QUEUE.size() > QUEUE_LIMIT)
    {
        return 1;
    }
    Sub_Channel_Item *item = new Sub_Channel_Item(channel, pFunc);
    _SUB_QUEUE.push(item);
    event_active(_sub_channel_ev, 0, 0);
    return 0;
}

int redis_mq::add_cal_queue(const char *channel)
{
    if (_CAL_QUEUE.size() > QUEUE_LIMIT)
    {
        return 1;
    }
    Cal_Channel_Item *item = new Cal_Channel_Item(channel);
    _CAL_QUEUE.push(item);
    event_active(_cal_channel_ev, 0, 0);
    return 0;
}

int redis_mq::pub_data_to_redis()
{
    auto item = _PUB_QUEUE.pop();
    if (item == NULL)
    {
        return 1;
    }
    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH %s %b", item->Channel.c_str(), item->Data.c_str(), item->Data.size());
    delete item;
    if (_PUB_QUEUE.size() > 0)
    {
        event_active(_pub_data_ev, 0, 0);
    }
    return 0;
}

int redis_mq::set_value_to_redis()
{
    auto item = _SET_QUEUE.pop();
    if (item == NULL)
    {
        return 1;
    }
    redisAsyncCommand(_pub_context, NULL, NULL, "SET %s %b", item->Channel.c_str(), item->Value.c_str(), item->Value.size());
    delete item;
    if (_SET_QUEUE.size() > 0)
    {
        event_active(_set_value_ev, 0, 0);
    }
    return 0;
}

int redis_mq::sub_channel_to_redis()
{
    auto item = _SUB_QUEUE.pop();
    if (item == NULL)
    {
        return 1;
    }
    redisAsyncCommand(_sub_context, Redis_Sub_Callback, (void *)item->Func, "SUBSCRIBE %s", item->Channel.c_str());
    delete item;
    if (_SUB_QUEUE.size() > 0)
    {
        event_active(_sub_channel_ev, 0, 0);
    }
    return 0;
}

int redis_mq::cal_channel_to_redis()
{
    auto item = _CAL_QUEUE.pop();
    if (item == NULL)
    {
        return 1;
    }
    redisAsyncCommand(_sub_context, NULL, NULL, "UNSUBSCRIBE %s", item->Channel.c_str());
    delete item;
    if (_CAL_QUEUE.size() > 0)
    {
        event_active(_cal_channel_ev, 0, 0);
    }
    return 0;
}

void *redis_mq::libevent_thread(void *arg)
{
    auto base = static_cast<event_base *>(arg);
    event_base_dispatch(base);
    return nullptr;
}

void redis_mq::Pub_to_redis_Event(evutil_socket_t fd, short what, void *arg)
{
    auto svr = static_cast<redis_mq *>(arg);
    svr->pub_data_to_redis();
}

void redis_mq::Set_to_redis_Event(evutil_socket_t fd, short what, void *arg)
{
    auto svr = static_cast<redis_mq *>(arg);
    svr->set_value_to_redis();
}

void redis_mq::Sub_to_redis_Event(evutil_socket_t fd, short what, void *arg)
{
    auto svr = static_cast<redis_mq *>(arg);
    svr->sub_channel_to_redis();
}

void redis_mq::Cal_to_redis_Event(evutil_socket_t fd, short what, void *arg)
{
    auto svr = static_cast<redis_mq *>(arg);
    svr->cal_channel_to_redis();
}

void redis_mq::Pub_Check_Event(evutil_socket_t fd, short what, void *arg)
{
}

void redis_mq::Sub_Check_Event(evutil_socket_t fd, short what, void *arg)
{
}

void redis_mq::Reconnect_Pub_Event(evutil_socket_t fd, short what, void *arg)
{
}

void redis_mq::Reconnect_Sub_Event(evutil_socket_t fd, short what, void *arg)
{
}

void redis_mq::Redis_Pub_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void redis_mq::Redis_Set_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void redis_mq::Redis_Sub_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto func = (datacallback)privdata;
    if (func == nullptr | reply == nullptr)
    {
        return;
    }
    if (reply->element[2]->type == REDIS_REPLY_INTEGER)
    {
    }
    else if (reply->element[2]->type == REDIS_REPLY_STRING)
    {
        func(reply->element[2]->str);
    }
}

void redis_mq::Redis_Cal_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void redis_mq::Redis_Sub_Check_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void redis_mq::Redis_Pub_Check_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void redis_mq::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
}

void redis_mq::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
}

// void redis_mq::Redis_Recv_Callback(redisAsyncContext *c, void *r, void *privdata)
// {
//     auto reply = static_cast<redisReply *>(r);

//     auto func = (datacallback)privdata;

//     if (func == nullptr | reply == nullptr)
//     {
//         return;
//     }

//     if (reply->element[2]->type == REDIS_REPLY_INTEGER)
//     {
//     }
//     else if (reply->element[2]->type == REDIS_REPLY_STRING)
//     {
//         func(reply->element[2]->str);
//     }
// }
