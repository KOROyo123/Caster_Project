/*
    // 完全遵循Ntrip2.0版本的实现
    // 由于1.0版本不完全符合HTTP协议，因此该类型不能完全支持1.0版本
    // 不支持Server采用1.0版本登录，支持用户采用1.0/2.0版本登录
*/
#pragma once

#include "ntrip_global.h"
#include "process_queue.h"

#include "knt/knt.h"
#include "knt/base64.h"

#include <event2/event.h>
#include <event2/http.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <string>
#include <memory>
#include <unordered_map>

class ntrip_common_listener
{
private:
    std::string listener_key; // 本机的IP和监听port得到的唯一key
    std::string _server_IP;   // 运行该程序的主机IP  和实际连接无关
    int _listen_port;
    int _connect_timeout = 0;
    bool _Server_Login_With_Password = false;
    bool _Client_Login_With_Password = false;
    bool _Nearest_Support = false;
    bool _Virtal_Support = false;

    event_base *_base;
    evhttp *_listener;
    evhttp_bound_socket * _handle;
    redisAsyncContext *_pub_context;

    std::shared_ptr<process_queue> _queue;
    std::unordered_map<std::string, json> _req_map;
    std::unordered_map<std::string, void *> *_connect_map;

public:
    ntrip_common_listener(event_base *base, std::shared_ptr<process_queue> queue, std::unordered_map<std::string, void *> *connect_map, redisAsyncContext *pub_context);
    ~ntrip_common_listener();

    int set_listen_conf(json conf);

    int start();
    int stop();

    int enable_Nearest_Support();
    int disable_Nearest_Support();

    int enable_Virtal_Support();
    int disable_Virtal_Support();
    int add_Virtal_Mount(std::string mount_point);
    int del_Virtal_Mount(std::string mount_point);

public:
    static void Ntrip_Decode_Request_cb(struct evhttp_request *req, void *arg);
    static void Ntrip_Source_Request_cb(struct evhttp_request *req, void *arg); // 获取源列表       GET /
    static void Ntrip_Client_Request_cb(struct evhttp_request *req, void *arg); // 对已有挂载点的访问回调  GET /XXXX
    static void Ntrip_Virtal_Request_cb(struct evhttp_request *req, void *arg);
    static void Ntrip_Nearest_Request_cb(struct evhttp_request *req, void *arg);
    static void Ntrip_Server_Request_cb(struct evhttp_request *req, void *arg);

    static void Redis_Verify_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Record_Callback(redisAsyncContext *c, void *r, void *privdata);

    static void Redis_Callback1(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback2(redisAsyncContext *c, void *r, void *privdata);

    int process_req(evhttp_request *req, int req_type);

private:
    std::string get_conncet_key(evhttp_request *req);

    int redis_Info_Verify(json req);
    int redis_Info_Record(json req);

    json decode_evhttp_req(evhttp_request *req);
    int login_authentication(json item);

    std::string decode_basic_authentication(std::string authentication);
};