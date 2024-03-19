#include "process_queue.h"
// #include "spdlog/spdlog.h"
// #include "knt/knt.h"

#define __class__ "process_queue"

process_queue::process_queue()
{
}

process_queue::~process_queue()
{
}

int process_queue::push(json req, int req_type)
{
    req["req_type"] = req_type;
    _queue.push(req);
    return 0;
}

json process_queue::poll()
{
    json req = _queue.front();
    _queue.pop();
    return req;
}

bool process_queue::not_null()
{
    if (_queue.size() == 0)
    {
        return false;
    }
    return true;
}

int process_queue::active_prrocesser()
{
    event_active(_processer, 0, 0);
    return 0;
}

int process_queue::push_and_active(json req, int req_type)
{
    push(req, req_type);
    return active_prrocesser();
}

int process_queue::add_processer(event *processer)
{
    _processer = processer;
    return 0;
}
