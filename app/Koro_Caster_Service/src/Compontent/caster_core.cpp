#include "caster_core.h"
#include <spdlog/spdlog.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#define __class__ "redis_msg_internal"

class redis_msg_internal
{
private:
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    event_base *_base;

public:
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_msg_internal(json conf, event_base *base);
    ~redis_msg_internal();

    int start();
    int stop();

    // Redis回调
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);
};

redis_msg_internal::redis_msg_internal(json conf, event_base *base)
{
    _redis_IP = conf["Redis_IP"];
    _redis_port = conf["Redis_Port"];
    _redis_Requirepass = conf["Redis_Requirepass"];
}

redis_msg_internal::~redis_msg_internal()
{
}

int redis_msg_internal::start()
{
    // 初始化redis连接
    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, _redis_IP.c_str(), _redis_port);
    struct timeval tv = {0};
    tv.tv_sec = 10;
    options.connect_timeout = &tv;

    _pub_context = redisAsyncConnectWithOptions(&options);
    if (_pub_context->err)
    {
        /* Let *c leak for now... */
        spdlog::error("redis eror: {}", _pub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_pub_context, _base);
    redisAsyncSetConnectCallback(_pub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_pub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_pub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    _sub_context = redisAsyncConnectWithOptions(&options);
    if (_sub_context->err)
    {
        /* Let *c leak for now... */
        spdlog::error("redis eror: {}", _sub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_sub_context, _base);
    redisAsyncSetConnectCallback(_sub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_sub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_sub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());
    return 0;
}

int redis_msg_internal::stop()
{
    redisAsyncDisconnect(_sub_context);
    redisAsyncFree(_sub_context);
    redisAsyncDisconnect(_pub_context);
    redisAsyncFree(_pub_context);
    return 0;
}

void redis_msg_internal::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: Connected to Redis Success", __class__);
}

void redis_msg_internal::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: redis info: Disconnected Redis", __class__);
}

redis_msg_internal *svr;

void recv_sub_channel_data_cb(struct redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto ctx = static_cast<std::pair<datacallback, void *> *>(privdata);

    auto Func = ctx->first;
    auto arg = ctx->second;

    Func(arg, reply->element[2]->str, reply->element[2]->len);
}

void check_mount_point_is_online_cb(struct redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto ctx = static_cast<std::pair<msgcallback, void *> *>(privdata);

    auto Func = ctx->first;
    auto arg = ctx->second;
    std::string result;

    if (reply->type != REDIS_REPLY_INTEGER)
    {
        return;
    }

    if (reply->integer == 1)
    {
        result = "true";
    }
    else
    {
        result = "false";
    }
    Func(arg, result.c_str(), result.size());
}

int CASTER::Init(json conf, event_base *base)
{
    svr = new redis_msg_internal(conf, base);
    svr->start();
    return 0;
}

int CASTER::Free()
{
    svr->stop();
    delete svr;
    return 0;
}

int CASTER::Clear()
{
    return 0;
}

int CASTER::Send_Common_Base_Online_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    auto context = svr->_pub_context;

    redisAsyncCommand(context, NULL, NULL, "HSET MPT_ONLINE:COMMON %s %s", mount_point, connect_key);

    return 0;
}

int CASTER::Send_Common_Base_Offline_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    auto context = svr->_pub_context;

    redisAsyncCommand(context, NULL, NULL, "HDEL MPT_ONLINE:COMMON %s %s", mount_point, connect_key);

    return 0;
}

int CASTER::Send_SYS_Relay_Base_Online_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    return 0;
}

int CASTER::Send_SYS_Relay_Base_Offline_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    return 0;
}

int CASTER::Send_TRD_Relay_Base_Online_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    return 0;
}

int CASTER::Send_TRD_Relay_Base_Offline_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    return 0;
}

int CASTER::Send_Rover_Online_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Send_Rover_Offline_Msg(const char *mount_point, const char *user_name, const char *connect_key)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Pub_Base_Raw_Data(const char *channel, const unsigned char *data, size_t data_length)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Sub_Base_Raw_Data(const char *channel, datacallback cb, void *arg)
{
    auto context = svr->_sub_context;

    auto ctx = new std::pair<datacallback, void *>(cb, arg);
    // redisAsyncCommand(context, check_mount_point_is_online_cb, arg, "HEXISTS MPT_ONLINE:COMMON %s", mount_point);

    return 0;
}

int CASTER::UnSub_Base_Raw_Data(const char *channel)
{
    auto context = svr->_sub_context;
    return 0;
}

int CASTER::Pub_Rover_Raw_Data(const char *channel, const unsigned char *data, size_t data_length)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Sub_Rover_Raw_Data(const char *channel, datacallback cb, void *arg)
{
    auto context = svr->_sub_context;
    return 0;
}

int CASTER::UnSub_Rover_Raw_Data(const char *channel)
{
    auto context = svr->_sub_context;
    return 0;
}

int CASTER::Get_Common_Source_List(msgcallback cb, void *arg)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Get_SYS_Relay_List(msgcallback cb, void *arg)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Get_TRD_Relay_List(msgcallback cb, void *arg)
{
    auto context = svr->_pub_context;
    return 0;
}

int CASTER::Check_Mount_Point_is_Online(const char *mount_point, msgcallback cb, void *arg)
{
    auto context = svr->_pub_context;

    auto ctx = new std::pair<msgcallback, void *>(cb, arg);
    redisAsyncCommand(context, check_mount_point_is_online_cb, arg, "HEXISTS MPT_ONLINE:COMMON %s", mount_point);

    return 0;
}

int CASTER::Check_Client_User_is_Online(const char *user_name, msgcallback cb, void *arg)
{
    auto context = svr->_pub_context;
    return 0;
}
