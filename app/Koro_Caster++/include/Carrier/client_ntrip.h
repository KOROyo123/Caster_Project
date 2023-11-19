#pragma once

#include "event2/event.h"



class client_ntrip
{
private:
    /* data */


    //一个redis连接


public:
    client_ntrip(/* args */);
    ~client_ntrip();

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);


    //挂载点转key
    
    //订阅指定基站




    //根据需求，订阅指定挂载点的消息
    //必要订阅，一个控制管道，接收控制消息


    //根据状态，向不同的话题发送消息（向状态统计发送自己的状态）


    //每个
    
};

