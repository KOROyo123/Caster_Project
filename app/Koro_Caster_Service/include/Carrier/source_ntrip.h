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

class source_ntrip
{
private:
    json _info;

    std::string _user_name;
    std::string _connect_key;
    std::string _ip;
    int _port;

    bool _NtripVersion2 = false;

    bufferevent *_bev;
    evbuffer *_evbuf;
    redisAsyncContext *_pub_context;
    std::shared_ptr<process_queue> _queue;

    std::string _list;

public:
    source_ntrip(json req, bufferevent *bev, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context);
    ~source_ntrip();

    int start();
    int stop();

    int set_source_list(std::string list);

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void WriteCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);

private:
    int build_source_table();
};
