#include "caster_core_internal.h"
#include <chrono>

#include <spdlog/spdlog.h>

redis_msg_internal::redis_msg_internal(json conf, event_base *base)
{
    _redis_IP = conf["Redis_IP"];
    _redis_port = conf["Redis_Port"];
    _redis_Requirepass = conf["Redis_Requirepass"];

    _base = base;
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

int redis_msg_internal::add_sub_cb_item(const char *channel, const char *connect_key, CasterCallback cb, void *arg)
{
    auto cb_item = new sub_cb_item();
    cb_item->cb = cb;
    cb_item->arg = arg;

    auto find = _sub_cb_map.find(channel);
    if (find == _sub_cb_map.end())
    {
        std::unordered_map<std::string, sub_cb_item *> channel_subs;
        _sub_cb_map.insert(std::pair(channel, channel_subs));
        find = _sub_cb_map.find(channel);
    }
    find->second.insert(std::pair(connect_key, cb_item));

    redisAsyncCommand(_sub_context, Redis_SUB_Callback, this, "SUBSCRIBE %s", channel);

    return 0;
}

int redis_msg_internal::del_sub_cb_item(const char *channel, const char *connect_key)
{
    auto find = _sub_cb_map.find(channel);
    if (find == _sub_cb_map.end())
    {
        // error
    }

    auto subs = find->second;
    auto item = subs.find(connect_key);
    if (item == subs.end())
    {
        // error
    }
    subs.erase(connect_key);
    redisAsyncCommand(_sub_context, NULL, NULL, "UNSUBSCRIBE %s", channel);
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

void redis_msg_internal::Redis_SUB_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto svr = static_cast<redis_msg_internal *>(privdata);

    if (!reply)
    {
        return;
    }
    if (reply->elements == 3)
    {
        auto re1 = reply->element[0];
        auto re2 = reply->element[1];
        auto re3 = reply->element[2];

        CatserReply Reply;
        Reply.type = CASTER_REPLY_STRING;
        Reply.str = re3->str;
        Reply.len = re3->len;

        auto channel_subs = svr->_sub_cb_map.find(re2->str); // 找到订阅该频道的map
        if (channel_subs == svr->_sub_cb_map.end())
        {
            return;
        }
        auto subs = channel_subs->second;
        auto size = subs.size();
        for (auto iter : subs)
        {
            auto cb_item = iter.second;
            auto Func = cb_item->cb;
            auto arg = cb_item->arg;
            Func(re2->str, arg, &Reply);
        }
    }
}

void redis_msg_internal::Redis_ONCE_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto ctx = static_cast<sub_cb_item *>(privdata);

    auto Func = ctx->cb;
    auto arg = ctx->arg;

    CatserReply Reply;
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        Reply.type = CASTER_REPLY_INTEGER;
        Reply.integer = reply->integer;
    }

    Func("", arg, &Reply);
}

long long redis_msg_internal::get_time_stamp()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // 转换为时间类型
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 获取秒数
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    long long seconds_count = seconds.count();
    return seconds_count;
}
