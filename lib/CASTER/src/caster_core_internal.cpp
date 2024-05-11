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

    _timeout_tv.tv_sec = 1;
    _timeout_tv.tv_usec = 0;
    _timeout_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_timeout_ev, &_timeout_tv);

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

int redis_msg_internal::add_local_active_connect_key(const char *connect_key)
{
    if (_local_active_channel.find(connect_key) == _local_active_channel.end())
    {
        _local_active_channel.insert(connect_key);
    }
    else
    {
        // 已经存在set中
    }
    return 0;
}

int redis_msg_internal::del_local_active_connect_key(const char *connect_key)
{
    if (_local_active_channel.find(connect_key) != _local_active_channel.end())
    {
        _local_active_channel.erase(connect_key);
        // redisAsyncCommand(_pub_context, NULL, NULL, "HSET MOUNT:ONLINE:COMMON %s", mount_point, connect_key);
    }
    else
    {
        // 在set中没有
    }
    return 0;
}

int redis_msg_internal::update_all_local_active_connect_key_expiration_time()
{
    




    return 0;
}

int redis_msg_internal::add_sub_cb_item(const char *channel, const char *connect_key, CasterCallback cb, void *arg)
{
    auto find = _sub_cb_map.find(channel);
    if (find == _sub_cb_map.end())
    {
        // 还没有订阅频道，添加订阅
        redisAsyncCommand(_sub_context, Redis_SUB_Callback, this, "SUBSCRIBE %s", channel);
        std::unordered_map<std::string, sub_cb_item> channel_subs;
        _sub_cb_map.insert(std::pair(channel, channel_subs));
    }
    find = _sub_cb_map.find(channel);

    // 更新订阅者列表
    redisAsyncCommand(_pub_context, NULL, NULL, "SADD CHANNEL:%s:SUBS %s", channel, connect_key);

    sub_cb_item cb_item;
    cb_item.cb = cb;
    cb_item.arg = arg;
    find->second.insert(std::pair(connect_key, cb_item));

    return 0;
}

int redis_msg_internal::del_sub_cb_item(const char *channel, const char *connect_key)
{
    auto channel_subs = _sub_cb_map.find(channel);
    if (channel_subs == _sub_cb_map.end())
    {
        return 1;
    }

    auto item = channel_subs->second.find(connect_key);
    if (item == channel_subs->second.end())
    {
        return 1;
    }
    // 更新订阅者列表
    redisAsyncCommand(_pub_context, NULL, NULL, "SREM CHANNEL:%s:SUBS %s", channel, connect_key);
    channel_subs->second.erase(item);

    return 0;
}

int redis_msg_internal::check_active_channel()
{
    auto cb_map = _sub_cb_map; // 先复制一份副本,采用副本进行操作，避免执行的回调函数对本体进行了操作，导致for循环出错
    for (auto channel_subs = cb_map.begin(); channel_subs != cb_map.end(); channel_subs++)
    // for (auto channel_subs : _sub_cb_map)
    {
        if (_active_channel.find(channel_subs->first) == _active_channel.end()) // 该订阅频道不在活跃频道中
        {
            if (channel_subs->second.size() == 0) // 订阅频道的实际用户为0
            {
                _sub_cb_map.erase(channel_subs->first); // 实际执行的操作是删除了原始记录
            }
            else
            {
                for (auto item = channel_subs->second.begin(); item != channel_subs->second.end(); item++)
                {
                    CatserReply Reply;
                    Reply.type = CASTER_REPLY_ERR;
                    auto cb_arg = item->second;
                    cb_arg.cb(channel_subs->first.c_str(), cb_arg.arg, &Reply);
                }
            }
        }
    }

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
        for (auto iter : channel_subs->second)
        {
            auto cb_item = iter.second;
            auto Func = cb_item.cb;
            auto arg = cb_item.arg;
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

    delete ctx;
}

void redis_msg_internal::Redis_Get_Hash_Field_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    // auto arg = static_cast<std::pair<redis_msg_internal *, std::set<std::string> *> *>(privdata);
    // auto svr = arg->first;
    // auto set = arg->second;
    auto set = static_cast<std::set<std::string> *>(privdata);

    if (!reply)
    {
        return;
    }

    set->clear();
    for (int i = 0; i < reply->elements; i += 2)
    {
        auto rep = reply->element[i];
        set->insert(reply->element[i]->str);
    }
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

std::string redis_msg_internal::get_source_list_text()
{
    return _source_list_text;
}

void redis_msg_internal::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto svr = static_cast<redis_msg_internal *>(arg);

    svr->update_source_list();

    svr->check_active_channel();
    svr->build_source_list();
}

int redis_msg_internal::update_source_list()
{
    // auto ctx0 = new std::pair<redis_msg_internal *, std::set<std::string> *>(this, &_online_common_mount);
    redisAsyncCommand(_pub_context, Redis_Get_Hash_Field_Callback, &_online_common_mount, "HGETALL MOUNT:ONLINE:COMMON");
    // redisAsyncCommand(_pub_context, Redis_Get_Source_Callback, &_online_sys_relay_mount, "HGETALL MOUNT:ONLINE:SYS_RELAY");
    // redisAsyncCommand(_pub_context, Redis_Get_Source_Callback, &_online_trd_relay_mount, "HGETALL MOUNT:ONLINE:TRD_RELAY");

    redisAsyncCommand(_pub_context, Redis_Get_Hash_Field_Callback, &_active_channel, "HGETALL CHANNEL:ACTIVE");

    return 0;
}

int redis_msg_internal::build_source_list()
{
    _source_list_text.clear();

    int size = 0;

    if (_send_nearest)
    {
        _nearest_items = convert_group_mount_to_string(_support_nearest_mount);
    }
    if (_send_virtual)
    {
        _virtual_items = convert_group_mount_to_string(_support_virtual_mount);
    }
    if (_send_sys_relay)
    {
        _sys_relay_items = convert_group_mount_to_string(_online_sys_relay_mount);
    }
    if (_send_common)
    {
        size = _online_common_mount.size();
        _common_items = convert_group_mount_to_string(_online_common_mount);
    }
    if (_send_trd_relay)
    {
        _trd_relay_items = convert_group_mount_to_string(_online_trd_relay_mount);
    }

    _source_list_text = _nearest_items +
                        _virtual_items +
                        _sys_relay_items +
                        _common_items +
                        _trd_relay_items;
    return 0;
}

std::string redis_msg_internal::convert_group_mount_to_string(std::set<std::string> group)
{
    std::string items;
    for (auto iter : group)
    {
        mount_info item;
        auto info = _mount_map.find(iter);
        if (info == _mount_map.end())
        {
            item = build_default_mount_info(iter);
        }
        else
        {
            item = info->second;
        }
        items += convert_mount_info_to_string(item);
    }
    return items;
}

std::string redis_msg_internal::convert_mount_info_to_string(mount_info i)
{
    std::string item;

    item = i.STR + ";" +
           i.mountpoint + ";" +
           i.identufier + ";" +
           i.format + ";" +
           i.format_details + ";" +
           i.carrier + ";" +
           i.nav_system + ";" +
           i.network + ";" +
           i.country + ";" +
           i.latitude + ";" +
           i.longitude + ";" +
           i.nmea + ";" +
           i.solution + ";" +
           i.generator + ";" +
           i.compr_encrryp + ";" +
           i.authentication + ";" +
           i.fee + ";" +
           i.bitrate + ";" +
           i.misc + ";" + "\r\n";

    return item;
}

mount_info redis_msg_internal::build_default_mount_info(std::string mount_point)
{
    // STR;              STR;
    // mountpoint;       KORO996;
    // identufier;       ShangHai;
    // format;           RTCM 3.3;
    // format-details;   1004(5),1074(1),1084(1),1094(1),1124(1)
    // carrier;          2
    // nav-system;       GPS+GLO+GAL+BDS
    // network;          KNT
    // country;          CHN
    // latitude;         36.11
    // longitude;        120.11
    // nmea;             0
    // solution;         0
    // generator;        SN
    // compr-encrryp;    none
    // authentication;   B
    // fee;              N
    // bitrate;          9600
    // misc;             caster.koroyo.xyz:2101/KORO996

    mount_info item = {
        "STR",
        mount_point,
        "unknown",
        "unknown",
        "unknown",
        "0",
        "unknown",
        "unknown",
        "unknown",
        "00.00",
        "000.00",
        "0",
        "0",
        "unknown",
        "unknown",
        "B",
        "N",
        "0000",
        "Not parsed or provided"};

    return item;
}
