#pragma once

#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>

#include <queue>
#include <list>
#include <mutex>
#include <memory>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class process_queue
{
private:
    std::queue<json> _queue;
    std::mutex _lock;

    event *_processer;

public:
    process_queue();
    ~process_queue();

    int add_processer(event *processer);

    int push(json req, int req_type);

    int active_prrocesser();

    int push_and_active(json req, int req_type);

    json poll();

    bool not_null();

    // int del_processer(event *process_event);
};
