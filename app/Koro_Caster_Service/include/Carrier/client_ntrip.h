/*
    用户已经上线的情况下，向redis写入用户登录信息
    判断当前已登录的用户数量，如果超过限制，启动下线流程
*/
#pragma once
#include "ntrip_global.h"
#include "process_queue.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>


#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class client_ntrip
{
private:
    json _info;

    std::string _mount_point;
    std::string _user_name;
    std::string _connect_key;
    std::string _ip;
    int _port;

    bool _NtripVersion2 = false;
    bool _transfer_with_chunked = false;

    bufferevent *_bev;
    evbuffer *_evbuf;

public:
    client_ntrip(json req, bufferevent *bev);
    ~client_ntrip();

    int start();//绑定回调，然后去AUTH添加登录记录（是否允许多用户登录由auth判断并处理），如果添加成功，那就发送reply给用户，然后通知CASTER上线，如果不成功，就进入关闭流程
    int stop();

    int bev_send_reply();

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);

    int data_transfer(evbuffer *evbuf);

    int publish_data_from_evbuf();
};
