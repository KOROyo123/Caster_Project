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

#include "Compontent/caster_core.h"

#include <spdlog/spdlog.h>

class data_transfer
{
private:
    // 一个用来接收tcp数据的bufferevent
    json _setting;
    std::string _transfer_mount;

    evbuffer *_evbuf;

    std::unordered_map<std::string, std::unordered_map<std::string, client_ntrip *>> _sub_map; // mount /connect_key/client_ntrip

public:
    data_transfer(json req);
    ~data_transfer();

    int start();
    int stop();

    int add_pub_server(std::string Mount_Point);
    int stop_all_sub_client(std::string Mount_Point); // 会依次关闭所有用户，当最后一个用户关闭时，会调用del_pub_server 关闭订阅

    bool mount_point_exist(std::string Mount_Point);
    int add_sub_client(std::string Mount_Point, std::string Connect_Key, client_ntrip *client);
    int del_sub_client(std::string Mount_Point, std::string Connect_Key);

    int transfer_date_to_sub(const char *mount_point, const char *data, size_t length);

private:
    int del_pub_server(std::string Mount_Point);
    // redisAsyncCommand(_pub_context, Redis_MountVerify_Callback, static_cast<void *>(this), "UNSUBSCRIBE STR_%s", "KOROYO2");
};
