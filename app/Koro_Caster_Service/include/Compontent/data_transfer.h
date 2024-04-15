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
#include "tcp_"

#include <spdlog/spdlog.h>

class data_transfer
{
private:
    // 一个用来接收tcp数据的bufferevent
    json _setting;
    std::string _transfer_mount;

    evbuffer *_evbuf;

    std::unordered_map<std::string, std::unordered_map<std::string, client_ntrip *>> _client_sub_map; // mount /connect_key/client_ntrip
    std::unordered_map<std::string, std::set<std::string>> _common_sub_map; // connect_key /connect_key/client_ntrip

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

public:

    int add_client_sub(std::string Mount_Point,std::string client_key, client_ntrip *client_obj);
    int del_client_sub(std::string Mount_Point,std::string client_key);
    int add_common_sub(std::string channel,std::string connect_key);
    int del_common_sub(std::string channel,std::string connect_key);

};
