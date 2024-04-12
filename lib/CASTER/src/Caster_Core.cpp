#include "Caster_Core.h"
#include <spdlog/spdlog.h>

#include "caster_core_internal.h"

redis_msg_internal *svr = nullptr;

int CASTER::Init(const char *json_conf, event_base *base)
{

    json conf = json::parse(json_conf);

    if (svr == nullptr)
    {
        svr = new redis_msg_internal(conf, base);
    }

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

int CASTER::Set_Base_Station_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    return 0;
}

int CASTER::Set_Base_Station_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type)
{
    return 0;
}

int CASTER::Check_Base_Station_is_Online(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
{
    return 0;
}

int CASTER::Pub_Base_Station_Raw_Data(const char *mount_point, const unsigned char *data, size_t data_length, Station_type type)
{
    return 0;
}

int CASTER::Sub_Base_Station_Raw_Data(const char *mount_point, CasterCallback cb, void *arg, Station_type type)
{
    return 0;
}

int CASTER::UnSub_Base_Station_Raw_Data(const char *mount_point, Station_type type)
{
    return 0;
}

int CASTER::Set_Rover_Client_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Client_type type)
{
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

int CASTER::Pub_Rover_Client_Raw_Data(const char *connect_key, const unsigned char *data, size_t data_length, Client_type type)
{
    return 0;
}

int CASTER::Sub_Rover_Client_Raw_Data(const char *connect_key, CasterCallback cb, void *arg, Client_type type)
{
    return 0;
}

int CASTER::UnSub_Rover_Client_Raw_Data(const char *connect_key, Client_type type)
{
    return 0;
}

int CASTER::Get_Source_Table_List(CasterCallback cb, void *arg, Source_type type)
{
    return 0;
}

int CASTER::Add_Source_Table_Item(const char *mount_point, const char * info, double lon, double lat, Source_type type)
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
