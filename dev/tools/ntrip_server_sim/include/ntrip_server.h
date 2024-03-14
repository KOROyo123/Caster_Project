#pragma once
#include "event.h"
#include "event2/bufferevent.h"
#include <string>

class ntrip_server
{
private:
    bufferevent *_bev;
    event_base *_base;

    timeval _ev = {1, 1};
    event *_sendev;

    std::string _mountpoint;
    std::string _user;
    std::string _pwd;
    std::string _userID;

    std::string _host;
    std::string _server;
    bool _ntrip2;

    evbuffer *_data_buf;

public:
    ntrip_server(event_base *base, evbuffer *data_buf);
    ~ntrip_server();

    static void ReadCallback(struct bufferevent *bev, void *arg);

    static void EventCallback(struct bufferevent *bev, short events, void *arg);

    static void VeifyCallback(struct bufferevent *bev, void *arg);
    static void ConnectCallback(struct bufferevent *bev, short events, void *arg);

    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);

    int connect(std::string mountpoint, std::string user,std::string pwd, std::string host, std::string server, bool ntrip2);
};
