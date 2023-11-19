#include "ntrip_caster.h"

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/http.h>

ntrip_caster::ntrip_caster(/* args */)
{

    auto_init();
}

ntrip_caster::ntrip_caster(std::shared_ptr<process_queue> queue)
{
    _queue = queue;

    auto_init();
}

ntrip_caster::~ntrip_caster()
{
}

int ntrip_caster::init()
{
    return 0;
}

int ntrip_caster::start()
{

    // 启动event_base处理线程
    start_server_thread();

    // 向消息处理队列中添加消息

    return 0;
}

int ntrip_caster::stop()
{
    return 0;
}

int ntrip_caster::request_process(json req)
{
    // 根据请求的类型，执行对应的操作

    // 判断json的类型

    // 执行对应的操作


    //新建类型
    create_client_ntrip(req);
    create_client_source(req);
    create_client_tcpcli(req);
    create_server_ntrip(req);
    create_server_relay(req);
    create_server_tcpcli(req);
    create_server_tcpsvr(req);

    ntrip_listen_to(req);
    tcpsvr_listen_to(req);
    ntrip_connect_to(req);
    tcpsvr_connect_to(req);



    //管理类型




    // 发布消息
    // 写入日志

    return 0;
}

int ntrip_caster::ntrip_listen_to(json req)
{

    // 创建一个listen对象，传入要监听的端口

    // 启动监听

    return 0;
}

void ntrip_caster::Request_Process_Cb(evutil_socket_t fd, short what, void *arg)
{
    ntrip_caster *svr = static_cast<ntrip_caster *>(arg);

    while (1)
    {
        json req = svr->_queue.get()->poll();

        if (req.is_null())
        {
            break;
        }

        svr->request_process(req);
    }
}

int ntrip_caster::auto_init()
{

    _base = event_base_new();
    _process_event = event_new(_base, -1, EV_PERSIST, Request_Process_Cb, this);

    if (_queue.get() == NULL)
    {
        std::shared_ptr<process_queue> newqueue(new process_queue());
        _queue = newqueue;
    }

    return 0;
}

int ntrip_caster::start_server_thread()
{

    pthread_t id;
    int ret = pthread_create(&id, NULL, event_base_thread, _base);
    if (ret)
    {
        return 1;
    }

    return 0;
}

void *ntrip_caster::event_base_thread(void *arg)
{

    event_base *base = (event_base *)arg;

    evthread_make_base_notifiable(base);

    event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

    event_base_dispatch(base);

    return nullptr;
}

int process_queue::push(json req)
{
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif

    _queue.push(req);

    return 0;
}

json process_queue::poll()
{

#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::mutex> lck(_lock);
#endif

    json req = _queue.front();

    _queue.pop();

    return req;
}
