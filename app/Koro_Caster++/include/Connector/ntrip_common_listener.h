#pragma once

#include "event2/http.h"


class ntrip_common_listener
{
private:
    evhttp *_listener;

    int _listen_port;

public:
    ntrip_common_listener(event_base *base);
    ~ntrip_common_listener();

    int set_listen_port(int port);

    int start();

    static void Ntrip_Common_Request_cb(struct evhttp_request *req, void *arg);
    static void Ntrip_Virtal_Request_cb(struct evhttp_request *req, void *arg);
};