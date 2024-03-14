/*
    用户已经上线的情况下，向redis写入用户登录信息
    判断当前已登录的用户数量，如果超过限制，启动下线流程
*/
#pragma once
#include "ntrip_global.h"
#include "process_queue.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class client_ntrip
{
private:
    json _info;

    std::string _mount_point;
    std::string _user_name;
    std::string _connect_key;
    std::string _ip;
    int _port;

    bool _NtripVersion2 = false;
    bool _transfer_with_chunked = false;

    evhttp_request *_hev;
    bufferevent *_bev;
    evbuffer *_evbuf;
    redisAsyncContext *_pub_context;
    std::shared_ptr<process_queue> _queue;

public:
    client_ntrip(json req, bufferevent *bev, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context);
    ~client_ntrip();

    int start();
    int stop();

    int hev_send_reply();
    int bev_send_reply();

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);

    static void Evhttp_Close_Callback(evhttp_connection *evcon, void *arg);

    int data_transfer(evbuffer *evbuf);

    int publish_data_from_evbuf();
};
