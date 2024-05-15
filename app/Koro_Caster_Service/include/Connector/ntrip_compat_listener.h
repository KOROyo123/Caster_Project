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

#include <string>
#include <memory>
#include <unordered_map>
#include <set>

class ntrip_compat_listener
{
private:
    int _listen_port;
    int _connect_timeout = 0;

    event_base *_base;
    evconnlistener *_listener;

    std::unordered_map<std::string, bufferevent *> *_connect_map;
    std::unordered_map<std::string, timeval *> _timer_map;

    std::set<std::string> _support_virtual_mount;

public:
    ntrip_compat_listener(json conf, event_base *base, std::unordered_map<std::string, bufferevent *> *connect_map);
    ~ntrip_compat_listener();

    int start();
    int stop();

public:
    // 新建连接相关
    static void AcceptCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg);
    static void AcceptErrorCallback(struct evconnlistener *listener, void *ctx);
    static void Ntrip_Decode_Request_cb(bufferevent *bev, void *arg);
    static void Bev_EventCallback(bufferevent *bev, short what, void *arg);

    // 解析请求相关（在解析完请求后，向AUTH验证用户名密码是否合法，只要合法就允许进入下一步（不判断是否已经登录，是否是重复登录，由后续步骤进行检查））
    int Process_GET_Request(bufferevent *bev, std::string connect_key, const char *path);
    int Process_POST_Request(bufferevent *bev, std::string connect_key, const char *path);
    int Process_SOURCE_Request(bufferevent *bev, std::string connect_key, const char *path, const char *secret);
    int Process_Unknow_Request(bufferevent *bev, std::string connect_key);

    // Auth验证回调
    static void Auth_Verify_Cb(const char *request, void *arg, AuthReply *reply);

private:
    // 内部函数
    // std::string get_conncet_key(bufferevent *bev);
    json decode_bufferevent_req(bufferevent *bev, std::string connect_key);
    std::string extract_path(std::string path);
    std::string extract_para(std::string path);
    std::string decode_basic_authentication(std::string authentication);
    int erase_and_free_bev(bufferevent *bev, std::string Connect_Key);
};
