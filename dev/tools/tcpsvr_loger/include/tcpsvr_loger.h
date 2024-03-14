#pragma once

#include <iostream>

#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include <event2/listener.h>
#include "spdlog/spdlog.h"

#include <arpa/inet.h>

#include <unordered_map>

struct port_bev
{
    int port;
    std::unordered_map<int, bufferevent *> bev_map;

    port_bev *sendto;
};

class tcpsvr_loger
{
private:
    event_base *_base;
    evbuffer *_glb_evbuf;
    port_bev _pb0, _pb1;
    evconnlistener *_listener[2];
    int _port[2];
    struct sockaddr_in _sin[2] = {0};

public:
    tcpsvr_loger(int port1, int port2);
    ~tcpsvr_loger();

    int start();

    static void ReadCallback(bufferevent *bev, void *arg);

    static void EventCallback(bufferevent *bev, short what, void *arg);

    static void AcceptCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg);
};
