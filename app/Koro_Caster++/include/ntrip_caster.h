#pragma once

#include "event2/util.h"
#include "event2/event.h"
#include "nlohmann/json.hpp"
#include <queue>
#include <list>
#include <mutex>

using json = nlohmann::json;

class process_queue
{
private:
    std::queue<json> _queue;
    std::mutex _lock;

    std::list<std::weak_ptr<event>> _processer;

public:
    int push();
    int poll();

    int active_one_prrocesser();

    int add_processer(event *process_event);
    int del_processer(event *process_event);
};

class ntrip_caster
{
private:
    //任务队列
    std::shared_ptr<process_queue> _queue;

    //process处理事件
    std::shared_ptr<event> _process_event;

    // redis连接

    // libevent 连接

public:
    ntrip_caster(/* args */);
    ~ntrip_caster();

    // 被动监听
    int ntrip_listen_to(int port); // evhttp        根据请求类型，创建对象  传入参数（GET:client_ntrip、client_source POST：server_ntrip）
    int tcpsvr_listen_to();        // bufferevent   根据请求端口，构建类型  传入参数（对于server_tcpsvr
    // 主动连接
    int ntrip_connect_to();  // evhttp？      根据请求类型，构建类型  创建对象传入fd、json
    int tcpsvr_connect_to(); // bufferevent   根据请求类型，构建类型  传入参数（对于server_tcpcli

    // 连接处理函数
    int request_process(json request);


public:
    //libevent回调

    static void Request_Process_Cb(evutil_socket_t fd, short what, void *arg);


private:

};
