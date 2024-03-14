#include "tcpsvr_loger.h"

tcpsvr_loger::tcpsvr_loger(int port1, int port2)
{



    _port[0] = port1;
    _port[1] = port2;

    for (int i = 0; i < 2; i++)
    {
        _sin[i].sin_family = AF_INET;
        _sin[i].sin_addr.s_addr = inet_addr("0.0.0.0");
        _sin[i].sin_port = htons(_port[i]);
    }

    _pb0.sendto = &_pb1;
    _pb0.port = _port[0];

    _pb1.sendto = &_pb0;
    _pb1.port = _port[1];
}

tcpsvr_loger::~tcpsvr_loger()
{
}

int tcpsvr_loger::start()
{
    _base = event_base_new();

    _listener[0] = evconnlistener_new_bind(_base, AcceptCallback, &_pb0, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, (struct sockaddr *)&_sin[0], sizeof(struct sockaddr_in));
    _listener[1] = evconnlistener_new_bind(_base, AcceptCallback, &_pb1, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, (struct sockaddr *)&_sin[1], sizeof(struct sockaddr_in));

    spdlog::info("link connect: {} <-> {}", _port[0], _port[1]);

    event_base_dispatch(_base);

    return 0;
}

void tcpsvr_loger::AcceptCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg)
{
    auto pb = static_cast<port_bev *>(arg);
    event_base *base = evconnlistener_get_base(listener);

    bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, ReadCallback, NULL, EventCallback, arg);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    pb->bev_map.insert(std::pair(fd, bev));
}

void tcpsvr_loger::ReadCallback(bufferevent *bev, void *arg)
{
    auto pb = static_cast<port_bev *>(arg);

    evbuffer *evbuf = evbuffer_new();

    bufferevent_read_buffer(bev, evbuf);

    int length = evbuffer_get_length(evbuf);

    char *data = new char[length + 1];
    data[length] = '\0';

    evbuffer_remove(evbuf, data, length);

    spdlog::info("{}->{}: {}", pb->port, pb->sendto->port, data);

    for (auto iter : pb->sendto->bev_map)
    {
        bufferevent_write(iter.second, data, length);
    }

    free(data);
}

void tcpsvr_loger::EventCallback(bufferevent *bev, short what, void *arg)
{

    auto pb = static_cast<port_bev *>(arg);

    int fd = bufferevent_getfd(bev);

    pb->bev_map.erase(fd);
}
