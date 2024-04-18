#include "Caster_Core.h"
#include <spdlog/spdlog.h>

#include "caster_core_internal.h"

redis_msg_internal *caster_svr = nullptr;

int CASTER::Init(const char *json_conf, event_base *base)
{
    json conf = json::parse(json_conf);

    if (caster_svr == nullptr)
    {
        caster_svr = new redis_msg_internal(conf, base);
        caster_svr->start();
    }
    return 0;
}

int CASTER::Free()
{
    caster_svr->stop();
    delete caster_svr;
    return 0;
}

int CASTER::Clear()
{
    // 清除与指定在线表（单实例部署）
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "DEL MOUNT:ONLINE:COMMON");
    return 0;
}

int CASTER::Clear(const char *server_key)
{
    // 清除与指定server_key(服务端口）相关的所有连接（暂未实现（用于集群部署）
    return 0;
}

int CASTER::Check_Mount_Type(const char *mount_point)
{
    return CASTER::STATION_COMMON;
}

int CASTER::Set_Base_Station_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "HSET MOUNT:ONLINE:COMMON %s %s", mount_point, connect_key);
    redisAsyncCommand(context, NULL, NULL, "HSET CHANNEL:ACTIVE MOUNT:%s %s", mount_point, connect_key);
    return 0;
}

int CASTER::Set_Base_Station_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "HDEL MOUNT:ONLINE:COMMON %s", mount_point);
    redisAsyncCommand(context, NULL, NULL, "HDEL CHANNEL:ACTIVE MOUNT:%s", mount_point);
    return 0;
}

int CASTER::Check_Base_Station_is_ONLINE(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
{
    auto context = caster_svr->_pub_context;
    auto ctx = new sub_cb_item();
    ctx->cb = cb;
    ctx->arg = arg;
    redisAsyncCommand(context, redis_msg_internal::Redis_ONCE_Callback, ctx, "HEXISTS MOUNT:ONLINE:COMMON %s", mount_point);
    return 0;
}

int CASTER::Pub_Base_Station_Raw_Data(const char *mount_point, const char *data, size_t data_length, const char *connect_key, Station_type type)
{
    auto context = caster_svr->_pub_context;
    std::string channel;
    channel += "MOUNT:";
    channel += mount_point;
    redisAsyncCommand(context, NULL, NULL, "PUBLISH %s %b", channel.c_str(), data, data_length);
    return 0;
}

int CASTER::Get_Base_Station_Sub_Num(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
{
    auto context = caster_svr->_pub_context;
    std::string channel;
    channel += "MOUNT:";
    channel += mount_point;
    auto ctx = new sub_cb_item();
    ctx->cb = cb;
    ctx->arg = arg;
    redisAsyncCommand(context, redis_msg_internal::Redis_ONCE_Callback, ctx, "SCARD CHANNEL:%s:SUBS", channel.c_str());
    return 0;
}

int CASTER::Sub_Base_Station_Raw_Data(const char *mount_point, const char *connect_key, CasterCallback cb, void *arg, Station_type type)
{
    std::string channel;
    channel += "MOUNT:";
    channel += mount_point;
    caster_svr->add_sub_cb_item(channel.c_str(), connect_key, cb, arg);
    return 0;
}

int CASTER::UnSub_Base_Station_Raw_Data(const char *mount_point, const char *connect_key, Station_type type)
{
    std::string channel;
    channel += "MOUNT:";
    channel += mount_point;
    caster_svr->del_sub_cb_item(channel.c_str(), connect_key);
    return 0;
}

int CASTER::Set_Rover_Client_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Client_type type)
{
    // auto context = caster_svr->_pub_context;
    // redisAsyncCommand(context, NULL, NULL, "HSET CLIENT:ONLINE:COMMON %s %s", mount_point, connect_key);
    return 0;
}

int CASTER::Set_Rover_Client_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Client_type type)
{
    return 0;
}

int CASTER::Check_Rover_Client_is_ONLINE(const char *user_name, CasterCallback cb, void *arg, Client_type type)
{
    return 0;
}

int CASTER::Pub_Rover_Client_Raw_Data(const char *client_key, const char *data, size_t data_length, const char *connect_key, Client_type type)
{
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "PUBLISH CLIENT:%s %b", client_key, data, data_length);
    return 0;
}

int CASTER::Sub_Rover_Client_Raw_Data(const char *client_key, CasterCallback cb, void *arg, const char *connect_key, Client_type type)
{
    std::string channel;
    channel += "CLIENT:";
    channel += client_key;
    caster_svr->add_sub_cb_item(channel.c_str(), connect_key, cb, arg);
    return 0;
}

int CASTER::Get_Rover_Client_Sub_Num(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
{
    auto context = caster_svr->_pub_context;
    std::string channel;
    channel += "CLIENT:";
    channel += mount_point;
    auto ctx = new sub_cb_item();
    ctx->cb = cb;
    ctx->arg = arg;
    redisAsyncCommand(context, redis_msg_internal::Redis_ONCE_Callback, ctx, "SCARD CHANNEL:%s:SUBS", channel.c_str());
    return 0;
}

int CASTER::UnSub_Rover_Client_Raw_Data(const char *client_key, const char *connect_key, Client_type type)
{
    std::string channel;
    channel += "CLIENT:";
    channel += client_key;
    caster_svr->del_sub_cb_item(channel.c_str(), connect_key);
    return 0;
}

int CASTER::Get_Source_Table_List(CasterCallback cb, void *arg, Source_type type)
{
    return 0;
}

int CASTER::Add_Source_Table_Item(const char *mount_point, const char *info, double lon, double lat, Source_type type)
{
    return 0;
}

int CASTER::Del_Source_Table_Item(const char *mount_point, Source_type type)
{
    return 0;
}

int CASTER::Get_Source_Table_Item(const char *mount_point, CasterCallback cb, void *arg, Source_type type)
{
    return 0;
}

int CASTER::Get_Radius_Table_List(double lon, double lat, CasterCallback cb, void *arg, Source_type type)
{
    return 0;
}

std::string CASTER::Get_Source_Table_Text()
{
    return caster_svr->get_source_list_text();
}
