/*
    // 兼容性listener，主要是需要支持Ntrip1.0协议部分不符合HTTP协议的部分
    // 基于nginx转发设置，可实现1.0版本和2.0版本同步使用


    jison格式

    req_type
    connect_key
    mount_point
    mount_para
    mount_group
    mount_info          STR STR: ;;;0;;;;;;0;0;;;N;N;
    user_name           Authorization
    user_pwd            Authorization
    user_baseID         Authorization
    user_agent          User-Agent
    ntrip_version       Ntrip-Version
    ntrip_gga           Ntrip-GGA
    http_chunked        Transfer-Encoding
    http_host           Host


*/
#pragma once

#include "ntrip_global.h"
#include "process_queue.h"

#include "knt/knt.h"
#include "knt/base64.h"

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <set>

class ntrip_compat_listener
{
private:
    std::string listener_key; // 本机的IP和监听port得到的唯一key
    std::string _server_IP;   // 运行该程序的主机IP  和实际连接无关
    int _listen_port;
    int _connect_timeout = 0;
    // bool _Server_Login_With_Password = false;
    // bool _Client_Login_With_Password = false;
    bool _Nearest_Support = false;
    bool _Virtal_Support = false;

    event_base *_base;
    evconnlistener *_listener;
    //redisAsyncContext *_pub_context;

    std::shared_ptr<process_queue> _queue;
    std::unordered_map<std::string, bufferevent *> *_connect_map;

    std::set<std::string> _support_virtual_mount;

public:
    ntrip_compat_listener(event_base *base, std::shared_ptr<process_queue> queue, std::unordered_map<std::string, bufferevent *> *connect_map);
    ~ntrip_compat_listener();

    int set_listen_conf(json conf);

    int start();
    int stop();

    //最近挂载点模式相关
    int enable_Nearest_Support();
    int disable_Nearest_Support();

    //虚拟挂载点相关
    int enable_Virtal_Support();
    int disable_Virtal_Support();
    int add_Virtal_Mount(std::string mount_point);
    int del_Virtal_Mount(std::string mount_point);

public:
    //新建连接相关
    static void AcceptCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg);
    static void AcceptErrorCallback(struct evconnlistener * listener, void * ctx);
    static void Ntrip_Decode_Request_cb(bufferevent *bev, void *arg);
    static void Bev_EventCallback(bufferevent *bev, short what, void *arg);

    //解析请求相关
    int Process_GET_Request(bufferevent *bev, const char *path);
    int Process_POST_Request(bufferevent *bev, const char *path);
    int Process_SOURCE_Request(bufferevent *bev, const char *path, const char *secret);
    int Process_Unknow_Request(bufferevent *bev);

    //处理请求相关
    void Ntrip_Source_Request_cb(bufferevent *bev, json req); // 获取源列表       GET /
    void Ntrip_Client_Request_cb(bufferevent *bev, json req); // 对已有挂载点的访问回调  GET /XXXX
    void Ntrip_Virtal_Request_cb(bufferevent *bev, json req);
    void Ntrip_Nearest_Request_cb(bufferevent *bev, json req);
    void Ntrip_Server_Request_cb(bufferevent *bev, json req);

private:
    //内部函数
    std::string get_conncet_key(bufferevent *bev);
    json decode_bufferevent_req(bufferevent *bev);
    std::string extract_path(std::string path);
    std::string extract_para(std::string path);
    std::string decode_basic_authentication(std::string authentication);
    int erase_and_free_bev(bufferevent *bev);
};
