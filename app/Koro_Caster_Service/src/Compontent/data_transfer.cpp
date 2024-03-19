#include "data_transfer.h"
#include <iostream>

#define __class__ "data_transfer"

data_transfer::data_transfer(json req, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context)
{

    _setting = req;

    _queue = queue;

    _sub_context = sub_context;

    _evbuf = evbuffer_new();
}

data_transfer::~data_transfer()
{

    evbuffer_free(_evbuf);
}

int data_transfer::start()
{
    redisAsyncCommand(_sub_context, Redis_PubOff_Callback, static_cast<void *>(this), "SUBSCRIBE mp_offline");
    return 0;
}

int data_transfer::stop()
{
    // redisAsyncCommand(_pub_context, Redis_UnSub_Callback, static_cast<void *>(this), "UNSUBSCRIBE STR_%s", _transfer_mount.c_str());

    // redisAsyncDisconnect(_sub_context);
    // redisAsyncFree(_sub_context);

    for (auto iter : _sub_map)
    {
        std::string mountpoint = iter.first;
        redisAsyncCommand(_sub_context, Redis_UnSub_Callback, static_cast<void *>(this), "UNSUBSCRIBE STR_%s", mountpoint.c_str());
        for (auto item : iter.second)
        {
            item.second->stop();
        }
    }

    // json req;
    // req["mount_point"] = _transfer_mount;

    // _queue->push_and_active(req, CLOSE_DATA_TRANSFER);

    // spdlog::trace("Data transfer: stop recv data and transfer");

    return 0;
}

int data_transfer::add_pub_server(std::string Mount_Point)
{
    std::unordered_map<std::string, client_ntrip *> item;
    _sub_map.insert(std::make_pair(Mount_Point, item));

    redisAsyncCommand(_sub_context, Redis_RecvSub_Callback, static_cast<void *>(this), "SUBSCRIBE STR_%s", Mount_Point.c_str());

    return 0;
}

int data_transfer::stop_all_sub_client(std::string Mount_Point)
{
    auto sub = _sub_map.find(Mount_Point);
    if (sub == _sub_map.end())
    {
        spdlog::error("can't find [{}] in data_transfer ", Mount_Point);
        return 1;
    }
    else
    {
        spdlog::info("Transfer: stop [{}] 's sub clients clients num: {}.", Mount_Point, sub->second.size());
        for (auto iter : sub->second)
        {
            iter.second->stop();
        }
    }
    return 0;
}

int data_transfer::del_pub_server(std::string Mount_Point)
{
    auto sub_cli = _sub_map.find(Mount_Point);

    if (sub_cli == _sub_map.end())
    {
        spdlog::error("data_transfer: {}: Mount Point [{}] is not sub in this data_transfer, del_pub_server fail", __func__, Mount_Point);
        return 1;
    }

    redisAsyncCommand(_sub_context, Redis_UnSub_Callback, static_cast<void *>(this), "UNSUBSCRIBE STR_%s", Mount_Point.c_str());

    _sub_map.erase(Mount_Point);

    return 0;
}

bool data_transfer::mount_point_exist(std::string Mount_Point)
{
    auto mp = _sub_map.find(Mount_Point);

    if (mp == _sub_map.end())
    {
        return false;
    }
    return true;
}

int data_transfer::add_sub_client(std::string Mount_Point, std::string Connect_Key, client_ntrip *client)
{
    auto sub_cli = _sub_map.find(Mount_Point);

    if (sub_cli == _sub_map.end())
    {
        // std::unordered_map<std::string, client_ntrip *> item;
        // item.insert(std::make_pair(Connect_Key, client));
        // _sub_map.insert(std::make_pair(Mount_Point, item));

        // // 还要添加订阅
        // redisAsyncCommand(_pub_context, Redis_RecvSub_Callback, static_cast<void *>(this), "SUBSCRIBE STR_%s", Mount_Point.c_str());

        spdlog::error("data_transfer:{} Mount Point [{}] is not sub in this data_transfer, add sub client fail", __func__, Mount_Point);
    }
    else
    {
        sub_cli->second.insert(std::make_pair(Connect_Key, client));
    }

    return 0;
}

int data_transfer::del_sub_client(std::string Mount_Point, std::string Connect_Key)
{
    auto sub_clis = _sub_map.find(Mount_Point);

    if (sub_clis == _sub_map.end())
    {
        spdlog::error("[{}:{}]: Mount Point [{}] is not in this data_transfer", __class__, __func__, Mount_Point);
        return 1;
    }

    auto one_cli = sub_clis->second.find(Connect_Key);
    if (one_cli == sub_clis->second.end())
    {
        spdlog::error("[{}:{}]: Connection [{}] is not in Mount Point [{}] 's sub_map, del_sub_client fail", __class__, __func__, Connect_Key, Mount_Point);
        return 0;
    }
    else
    {
        sub_clis->second.erase(Connect_Key);
    }

    if (sub_clis->second.size() == 0)
    {
        spdlog::debug("[{}:{}]: Mount Point [{}] 's sub_map is 0, del this pub_server ", __class__, __func__, Mount_Point);
        del_pub_server(Mount_Point);
    }

    return 0;
}

void data_transfer::Redis_PubOff_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    if (r == nullptr)
    {
        return;
    }

    auto reply = static_cast<redisReply *>(r);
    auto svr = static_cast<data_transfer *>(privdata);

    // for (int i = 0; i < reply->elements; i++)
    // {
    //     auto ele = reply->element[i];

    //     switch (ele->type)
    //     {
    //     case REDIS_REPLY_STRING:
    //         std::cout << ele->str << std::endl;
    //         break;

    //     case REDIS_REPLY_DOUBLE:
    //         break;

    //     case REDIS_REPLY_INTEGER:
    //         std::cout << ele->integer << std::endl;
    //         break;

    //     default:
    //         break;
    //     }
    // }

    auto ele = reply->element[2];
    if (ele->type == REDIS_REPLY_STRING)
    {
        if (svr->mount_point_exist(ele->str))
        {
            svr->stop_all_sub_client(ele->str);
        }
    }
}

void data_transfer::Redis_RecvSub_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    if (r == nullptr)
    {
        return;
    }

    auto reply = static_cast<redisReply *>(r);
    auto svr = static_cast<data_transfer *>(privdata);

    svr->transfer_date_to_sub(reply, reply->elements);
}

void data_transfer::Redis_UnSub_Callback(redisAsyncContext *c, void *r, void *privdata)
{
}

void data_transfer::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("redis eror: {}", c->errstr);
        return;
    }
    spdlog::trace("redis info: Connected to Redis Success");
}

void data_transfer::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("redis eror: {}", c->errstr);
        return;
    }
    spdlog::trace("redis info: Disconnected Redis");
}

int data_transfer::transfer_date_to_sub(redisReply *reply, int reply_num)
{

    char mount[128] = {'\0'};

    sscanf(reply->element[1]->str, "STR_%s", mount);

    auto item = _sub_map.find(mount);

    evbuffer_add(_evbuf, reply->element[2]->str, reply->element[2]->len);

    if (item == _sub_map.end())
    {
        spdlog::debug("[{}:{}]: Receive {} data , but not in _sub_map", __class__, __func__, mount);
        return 1;
    }

    auto clients = item->second;
    for (auto iter : clients)
    {
        evbuffer *trans_evbuf = evbuffer_new();
        evbuffer_add_buffer_reference(trans_evbuf, _evbuf);
        iter.second->data_transfer(trans_evbuf);
        evbuffer_free(trans_evbuf);
    }

    evbuffer_drain(_evbuf, evbuffer_get_length(_evbuf));
    return 0;
}
