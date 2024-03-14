#pragma once
#include <string>

#include "hiredis.h"

// #include "rtklib.h"

// #include "spdlog/spdlog.h"

#include <event2/event.h>
#include <event2/thread.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <mutex>
#include <queue>
#include <set>

typedef int (*datacallback)(const char *szdata);

class Pub_Data_Item
{
public:
    Pub_Data_Item(std::string channel, std::string data)
    {
        Channel = channel;
        Data = data;
    }

    std::string Channel;
    std::string Data;
};

class Set_Value_Item
{
public:
    Set_Value_Item(std::string channel, std::string value)
    {
        Channel = channel;
        Value = value;
    }

    std::string Channel;
    std::string Value;
};

class Sub_Channel_Item
{
public:
    Sub_Channel_Item(std::string channel, datacallback func)
    {
        Channel = channel;
        Func = func;
    }

    std::string Channel;
    datacallback Func;
};

class Cal_Channel_Item
{
public:
    Cal_Channel_Item(std::string channel)
    {
        Channel = channel;
    }
    std::string Channel;
};

template <typename T>
class redis_queue
{
public:
    redis_queue();
    ~redis_queue();

    void push(T item);
    T pop();
    int size();

private:
    std::mutex _lock;
    std::queue<T> _que;
};

class redis_mq
{
    // redis连接设置
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    int QUEUE_LIMIT = 500;

    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

    event_base *_pub_base;
    event_base *_sub_base;

    redis_queue<Pub_Data_Item *> _PUB_QUEUE;
    redis_queue<Set_Value_Item *> _SET_QUEUE;
    redis_queue<Sub_Channel_Item *> _SUB_QUEUE;
    redis_queue<Cal_Channel_Item *> _CAL_QUEUE;

public:
    redis_mq();
    ~redis_mq();

    int init(const char *ip, int port, const char *pwd);

    int start();
    int stop();

    int add_pub_queue(const char *channel, const char *data);
    int add_set_queue(const char *channel, const char *data);
    int add_sub_queue(const char *channel, int (*pFunc)(const char *szdata));
    int add_cal_queue(const char *channel);

private:
    // event处理函数
    int pub_data_to_redis();
    int set_value_to_redis();
    int sub_channel_to_redis();
    int cal_channel_to_redis();

private:
    // 发布事件
    event *_pub_data_ev;
    event *_set_value_ev;
    event *_sub_channel_ev;
    event *_cal_channel_ev;
    // 重连事件
    event *_reconnect_pub_ev;
    event *_reconnect_sub_ev;
    // 连接状态检查
    event *_pub_con_check_ev;
    event *_sub_con_check_ev;
    timeval _pub_con_check_tv;
    timeval _sub_con_check_tv;

public:
    static void *libevent_thread(void *arg);
    // event事件
    // 发布订阅
    static void Pub_to_redis_Event(evutil_socket_t fd, short what, void *arg);
    static void Set_to_redis_Event(evutil_socket_t fd, short what, void *arg);
    static void Sub_to_redis_Event(evutil_socket_t fd, short what, void *arg);
    static void Cal_to_redis_Event(evutil_socket_t fd, short what, void *arg);
    // 检测连接
    static void Pub_Check_Event(evutil_socket_t fd, short what, void *arg);
    static void Sub_Check_Event(evutil_socket_t fd, short what, void *arg);
    // 重连
    static void Reconnect_Pub_Event(evutil_socket_t fd, short what, void *arg);
    static void Reconnect_Sub_Event(evutil_socket_t fd, short what, void *arg);

    // redis回调
    // 发布订阅回调
    static void Redis_Pub_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Set_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Sub_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Cal_Callback(redisAsyncContext *c, void *r, void *privdata);
    // 检查连接回调
    static void Redis_Sub_Check_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Pub_Check_Callback(redisAsyncContext *c, void *r, void *privdata);
    // 连接
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);
};

template <typename T>
inline redis_queue<T>::redis_queue()
{
}

template <typename T>
inline redis_queue<T>::~redis_queue()
{
    while (!_que.empty())
    {
        auto item = _que.front();
        delete item;
        _que.pop();
    }
}

template <typename T>
inline void redis_queue<T>::push(T item)
{
    std::lock_guard<std::mutex> lock(_lock);
    _que.push(item);
}

template <typename T>
inline T redis_queue<T>::pop()
{
    std::lock_guard<std::mutex> lock(_lock);
    if (_que.empty())
    {
        return NULL;
    }
    auto item = _que.front();
    _que.pop();
    return item;
}

template <typename T>
inline int redis_queue<T>::size()
{
    std::lock_guard<std::mutex> lock(_lock);
    return _que.size();
}
