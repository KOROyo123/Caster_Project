#pragma once

#include "event2/event.h"



class client_ntrip
{
private:
    /* data */
public:
    client_ntrip(/* args */);
    ~client_ntrip();

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);


    //挂载点转key
    
    //订阅指定基站

    
};

