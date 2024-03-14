#include "process_queue.h"

#define __class__ "process_queue"

process_queue::process_queue()
{
}

process_queue::~process_queue()
{
}

int process_queue::push(json req, int req_type)
{
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif
    req["req_type"] = req_type;
    _queue.push(req);
    //spdlog::trace("process_queue: push req, type:{},queue size:{}", req_type, _queue.size());
    return 0;
}

json process_queue::poll()
{
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif

    json req = _queue.front();
    _queue.pop();
    //spdlog::trace("process_queue: pop req,queue size:{}", _queue.size());
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
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif
    push(req, req_type);
    return active_prrocesser();
}

int process_queue::add_processer(event *processer)
{
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif
    _processer = processer;
    return 0;
}
