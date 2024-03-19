/*
    json 格式

    标准格式




*/
#pragma once
#include "process_queue.h"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "ntrip_global.h"
#include "knt/knt.h"
#include "knt/base64.h"

// 连接建立成功，后发送验证信息，验证通过之后，创建一个创建relay_server的请求

class ntrip_relay_connector
{
private:
    event_base *_base;

    redisAsyncContext *_pub_context;

    std::shared_ptr<process_queue> _queue;
    std::unordered_map<std::string, json> _req_map;//这个req_map只有插入操作没有删除操作，后续应该修复！
    std::unordered_map<std::string, bufferevent *> *_connect_map;

public:
    ntrip_relay_connector(event_base *base, std::shared_ptr<process_queue> queue, std::unordered_map<std::string, bufferevent *> *connect_map, redisAsyncContext *context);
    ~ntrip_relay_connector();

    int start();
    int stop();

    std::string create_new_connection(json con_info);

    static void EventCallback(struct bufferevent *bev, short events, void *arg);
    static void ReadCallback(struct bufferevent *bev, void *arg);

    int send_login_request(bufferevent *bev, std::string Conncet_Key);
    int verify_login_response(bufferevent *bev, std::string Conncet_Key);

    int request_new_relay_server(std::string Conncet_Key);
    int request_give_back_account(std::string Conncet_Key);

private:
    //更新redis记录
    int redis_Info_Record(json req);


};
