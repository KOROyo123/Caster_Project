#pragma once
#include"event.h"
#include "event2/bufferevent.h"
#include <string>


class ntrip_client
{
private:
    bufferevent *_bev;
    event_base *_base;

    std::string _mountpoint;
    std::string _userID;

    std::string _host;
    std::string _server;
    bool _ntrip2;

public:
    ntrip_client(event_base *base);
    ~ntrip_client();

    static void ReadCallback(struct bufferevent *bev, void *arg);

    static void EventCallback(struct bufferevent *bev, short events, void *arg);

    static void VeifyCallback(struct bufferevent *bev, void *arg);
    static void ConnectCallback(struct bufferevent *bev, short events, void *arg);

    static void TimeoutCallback(evutil_socket_t *bev, short events, void *arg);

    int connect(std::string mountpoint, std::string userID, std::string host, std::string server, bool ntrip2);

    int send_login_msg();

    int decode_verify_msg();
};



