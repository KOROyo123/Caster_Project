#pragma once
#include "ntrip_global.h"
#include "process_queue.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>


#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class source_ntrip
{
private:
    json _info;

    std::string _connect_key;
    std::string _user_name;
    std::string _ip;
    int _port;

    bool _NtripVersion2 = false;

    bufferevent *_bev;
    evbuffer *_evbuf;

    std::string _source_list;

public:
    source_ntrip(json req, bufferevent *bev);
    ~source_ntrip();

    int start();
    int stop();

    static void WriteCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);

private:
    int build_source_table();
};
