#pragma once

#include "event2/util.h"
#include "event2/event.h"
#include <event2/http.h>
#include "nlohmann/json.hpp"
#include <queue>
#include <list>
#include <mutex>
#include <memory>

#include"Connector/ntrip_common_listener.h"
#include"Connector/ntrip_relay_connector.h"
#include"Connector//tcpcli_relay_connector.h"
#include"Connector/tcpsvr_common_listener.h"

#include"Carrier/client_ntrip.h"
#include"Carrier/client_source.h"
#include"Carrier/client_tcpcli.h"
#include"Carrier/server_ntrip.h"
#include"Carrier/server_relay.h"
#include"Carrier/server_tcpcli.h"
#include"Carrier/server_tcpsvr.h"


using json = nlohmann::json;

class process_queue
{
private:
    std::queue<json> _queue;
    std::mutex _lock;

    std::list<std::weak_ptr<event>> _processer;

public:
    int push(json req);
    json poll();

    int active_one_prrocesser();

    int add_processer(event *process_event);
    int del_processer(event *process_event);
};

class ntrip_caster
{
private:
    std::shared_ptr<json> _global_config;

    // 任务队列
    std::shared_ptr<process_queue> _queue;

    // process处理事件
    event *_process_event;

    // redis连接

    // libevent 连接

    // event_base 一个对象只有一个，运行在一个独立线程中
    event_base *_base;

public:
    ntrip_caster(/* args */);
    ntrip_caster(std::shared_ptr<process_queue> _queue);
    ~ntrip_caster();

    int init();

    int start();

    int stop();

    // 创建四种连接器（连接器完成连接建立的消息验证工作，提取连接类型，创建对应的传输器
    int ntrip_listen_to(json req);  // evhttp        根据请求类型，创建对象  传入参数（GET:client_ntrip、client_source POST：server_ntrip）
    int tcpsvr_listen_to(json req); // bufferevent   根据请求端口，构建类型  传入参数（对于server_tcpsvr
    int ntrip_connect_to(json req);  // evhttp？      根据请求类型，构建类型  创建对象传入fd、json
    int tcpsvr_connect_to(json req); // bufferevent   根据请求类型，构建类型  传入参数（对于server_tcpcli

    // 创建七种传输器（不同的传输器根据不同连接的特性，负责数据的收发工作）
    int create_client_ntrip(json req);//用Ntrip协议登录的用户
    int create_client_source(json req);//用Ntrip协议获取源列表
    int create_client_tcpcli(json req);//用TCP协议获取数据的用户
    int create_server_ntrip(json req);//基站主动接入产生的数据源
    int create_server_relay(json req);//主动连接其他caster的数据源
    int create_server_tcpcli(json req);//主动连接其他tcpsvr产生的数据源
    int create_server_tcpsvr(json req);//TCP主动接入产生的数据源

    //连接管理函数(对已经创建的连接执行关闭连接，修改连接等操作)
    int process_link_manage(json req);

    // 连接处理函数
    int request_process(json req);


    //连接控制器
private:

    //监听器map

    //传输器map

    //对于每一个连接，都会记录到内部中，当需要寻找的时候，可以通过记录来找到每一个连接，调用对应的方法

private:
    int auto_init();

public:
    int start_server_thread();

    static void *event_base_thread(void *arg);

    // // libevent回调

    // 内部任务队列处理
    static void Request_Process_Cb(evutil_socket_t fd, short what, void *arg);

    // // Ntrip连接处理（被动）
    // static void Ntrip_Common_Request_cb(struct evhttp_request *req, void *arg);
    // static void Ntrip_Virtal_Request_cb(struct evhttp_request *req, void *arg);
    // // TCP被动连接
    // static void TCP_Common_Request_Connect_Cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg);
    // static void TCP_Common_Request_ConError_Cb(struct evconnlistener *listener, void *arg);

    // // 主动连接相关回调(eventcb)
    // static void Ntrip_Relay_Request_cb(struct bufferevent *bev, short what, void *arg);
    // // TCP主动连接(eventcb)
    // static void TCP_Relay_Request_cb(struct bufferevent *bev, short what, void *arg);
};
