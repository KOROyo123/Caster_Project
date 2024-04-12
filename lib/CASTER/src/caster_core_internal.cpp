#include "caster_core_internal.h"

#include <spdlog/spdlog.h>

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