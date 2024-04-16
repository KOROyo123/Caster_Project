#include "process_queue.h"
// #include "spdlog/spdlog.h"
// #include "knt/knt.h"

#define __class__ "process_queue"

class process_queue_intenal
{
private:
    std::queue<json> _queue;
    event *_processer;

public:
    process_queue_intenal();
    ~process_queue_intenal();

    int add_processer(event *processer);

    int push(json req);

    int active_prrocesser();

    int push_and_active(json req);

    json pop();

    bool not_null();

    // int del_processer(event *process_event);
};

process_queue_intenal::process_queue_intenal()
{
}

process_queue_intenal::~process_queue_intenal()
{
}

int process_queue_intenal::push(json req)
{
    _queue.push(req);
    return 0;
}

json process_queue_intenal::pop()
{
    json req = _queue.front();
    _queue.pop();
    return req;
}

bool process_queue_intenal::not_null()
{
    if (_queue.size() == 0)
    {
        return false;
    }
    return true;
}

int process_queue_intenal::active_prrocesser()
{
    event_active(_processer, 0, 0);
    return 0;
}

int process_queue_intenal::push_and_active(json req)
{
    push(req);
    return active_prrocesser();
}

int process_queue_intenal::add_processer(event *processer)
{
    _processer = processer;
    return 0;
}

//
process_queue_intenal *queue_svr = nullptr;

int QUEUE::Init(event *process_event)
{
    queue_svr = new process_queue_intenal();
    queue_svr->add_processer(process_event);
    return 0;
}

int QUEUE::Free()
{
    delete queue_svr;
    return 0;
}

int QUEUE::Push(json req)
{
    return queue_svr->push_and_active(req);
}

json QUEUE::Pop()
{
    return queue_svr->pop();
}

bool QUEUE::Active()
{
    return queue_svr->active_prrocesser();
}

bool QUEUE::Not_Null()
{
    return queue_svr->not_null();
}
