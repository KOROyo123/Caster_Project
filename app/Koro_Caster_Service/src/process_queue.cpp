#include "process_queue.h"
// #include "spdlog/spdlog.h"
// #include "knt/knt.h"

#define __class__ "process_queue"


class process_queue
{
private:
    std::queue<json> _queue;
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

int QUEUE::Init(event *process_event)
{
    return 0;
}

int QUEUE::Free()
{
    return 0;
}

int QUEUE::Push(json req, int req_type)
{
    return 0;
}

json QUEUE::Pop()
{
    return json();
}

bool QUEUE::Active()
{
    return false;
}

bool QUEUE::Not_Null()
{
    return false;
}
