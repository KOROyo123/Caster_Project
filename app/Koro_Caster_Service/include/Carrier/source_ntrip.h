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
    /* data */
public:
    source_ntrip(json req, bufferevent *bev, std::shared_ptr<process_queue> queue,redisAsyncContext *sub_context, redisAsyncContext *pub_context);
    ~source_ntrip();
};


