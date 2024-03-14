/*
    // 记录负责发送的挂载点

    // 记录要传给的client对象？ bufferevent，和用户名？用户名重复怎么办？怎么删除？

    // IP端口+IP端口，来定义唯一的连接(通过TCP四原组得到)
    // ConnectKey:C0A801A71FBDC0A801A71FBC
    // base64
    // C0A801A7

*/
#pragma once

#include "ntrip_global.h"
#include "client_ntrip.h"

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <spdlog/spdlog.h>


class data_transfer
{
private:
    // 一个用来接收tcp数据的bufferevent

    json _setting;
    std::string _transfer_mount;

    evbuffer *_evbuf;
    redisAsyncContext *_sub_context;

    std::unordered_map<std::string, std::unordered_map<std::string, client_ntrip *>> _sub_map; // mount /connect_key/client_ntrip
    std::unordered_map<std::string, client_ntrip *> _sub_client;
    std::shared_ptr<process_queue> _queue;

public:
    data_transfer(json req, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context);
    ~data_transfer();

    int start();
    int stop();

    int add_pub_server(std::string Mount_Point);
    int stop_all_sub_client(std::string Mount_Point);  //会依次关闭所有用户，当最后一个用户关闭时，会调用del_pub_server 关闭订阅


    bool mount_point_exist(std::string Mount_Point);
    int add_sub_client(std::string Mount_Point, std::string Connect_Key, client_ntrip *client);
    int del_sub_client(std::string Mount_Point, std::string Connect_Key);

    static void Redis_PubOff_Callback(redisAsyncContext *c, void *r, void *privdata);

    static void Redis_RecvSub_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_UnSub_Callback(redisAsyncContext *c, void *r, void *privdata);

    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

    int transfer_date_to_sub(redisReply *data, int reply_num);

private:
    int del_pub_server(std::string Mount_Point);
    // redisAsyncCommand(_pub_context, Redis_MountVerify_Callback, static_cast<void *>(this), "UNSUBSCRIBE STR_%s", "KOROYO2");
};
