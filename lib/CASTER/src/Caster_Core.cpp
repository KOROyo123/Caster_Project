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
    return 0;
}

int CASTER::Set_Base_Station_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "HSET MOUNT:ONLINE:COMMON %s %s", mount_point, connect_key);
    return 0;
}

int CASTER::Set_Base_Station_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    auto context = caster_svr->_pub_context;
    redisAsyncCommand(context, NULL, NULL, "HDEL MOUNT:ONLINE:COMMON %s", mount_point);
    return 0;
}

int CASTER::Check_Base_Station_is_Online(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
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

int CASTER::Check_Rover_Client_is_Online(const char *user_name, CasterCallback cb, void *arg, Client_type type)
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
    return std::string();
}
