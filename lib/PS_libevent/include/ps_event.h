#pragma once


#include <string>

#include "event2/bufferevent.h"



class ps_event
{
private:
    /* data */
public:
    ps_event(/* args */);
    ~ps_event();


    //创建一个发布者

    //删除一个发布者

    //创建一个订阅者

    //删除一个订阅者

    //订阅

    //取消订阅

    //发布


    //主线程  启动
    int start();

};

ps_event::ps_event(/* args */)
{
}

ps_event::~ps_event()
{
}
