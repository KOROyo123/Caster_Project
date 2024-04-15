#define __class__ "redis_msg_internal"
#include "Caster_Core.h"
#include <string>
#include <unordered_map>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class sub_cb_item
{
public:
    CasterCallback cb;
    void *arg;
};

class redis_msg_internal
{
private:
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    event_base *_base;

    std::unordered_map<std::string, std::unordered_map<std::string, sub_cb_item *>> _sub_cb_map; // channel/connect_key/cb_arg

public:
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_msg_internal(json conf, event_base *base);
    ~redis_msg_internal();

    int start();
    int stop();

    int add_sub_cb_item(const char *channel, const char *connect_key, CasterCallback cb, void *arg);
    int del_sub_cb_item(const char *channel, const char *connect_key);

    // Redis回调
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

    static void Redis_SUB_Callback(redisAsyncContext *c, void *r, void *privdata);

    static long long get_time_stamp();
};
