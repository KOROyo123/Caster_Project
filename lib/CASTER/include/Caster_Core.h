#pragma once
#include <event2/event.h>
#include <string>

#define CASTER_REPLY_ERR -1
#define CASTER_REPLY_OK 0

#define CASTER_REPLY_STRING 1
#define CASTER_REPLY_ARRAY 2
#define CASTER_REPLY_INTEGER 3
#define CASTER_REPLY_NIL 4

struct CatserReply
{
    int type;
    char *str;
    size_t len;
    int integer;
};

struct mount_info
{
    std::string STR;
    std::string mountpoint;
    std::string identufier;
    std::string format;
    std::string format_details;
    std::string carrier;
    std::string nav_system;
    std::string network;
    std::string country;
    std::string latitude;
    std::string longitude;
    std::string nmea;
    std::string solution;
    std::string generator;
    std::string compr_encrryp;
    std::string authentication;
    std::string fee;
    std::string bitrate;
    std::string misc;
};

typedef void (*CasterCallback)(const char *request, void *arg, CatserReply *reply);

namespace CASTER
{

    enum Station_type
    {
        STATION_COMMON = 1,
        STATION_SYS_RELAY,
        STATION_TRD_RELAY,

    };

    enum Client_type
    {
        CLIENT_COMMON = 1,
        CLIENT_SYS_RELAY,
        CLIENT_TRD_RELAY,
    };

    enum Source_type
    {
        SOURCE_COMMON = 1,
        SOURCE_SYS_RELAY,
        SOURCE_TRD_RELAY,
    };

    int Init(const char *json_conf, event_base *base);
    int Free();

    int Clear();
    int Clear(const char *server_key);

    int Set_Base_Station_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type = STATION_COMMON);
    int Set_Base_Station_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Station_type type = STATION_COMMON);
    int Check_Base_Station_is_Online(const char *mount_point, CasterCallback cb, void *arg, Station_type type = STATION_COMMON);

    int Pub_Base_Station_Raw_Data(const char *mount_point, const char *data, size_t data_length, const char *connect_key = "", Station_type type = STATION_COMMON); // 同时延长BaseStation State的有效期
    int Sub_Base_Station_Raw_Data(const char *mount_point, const char *connect_key, CasterCallback cb, void *arg, Station_type type = STATION_COMMON);
    int UnSub_Base_Station_Raw_Data(const char *mount_point, const char *connect_key, Station_type type = STATION_COMMON);

    int Set_Rover_Client_State_ONLINE(const char *mount_point, const char *user_name, const char *connect_key, Client_type type = CLIENT_COMMON);
    int Set_Rover_Client_State_OFFLINE(const char *mount_point, const char *user_name, const char *connect_key, Client_type type = CLIENT_COMMON);
    int Check_Rover_Client_is_Online(const char *user_name, CasterCallback cb, void *arg, Client_type type = CLIENT_COMMON);

    int Pub_Rover_Client_Raw_Data(const char *client_key, const char *data, size_t data_length, const char *connect_key = "", Client_type type = CLIENT_COMMON);
    int Sub_Rover_Client_Raw_Data(const char *client_key, CasterCallback cb, void *arg, const char *connect_key = "", Client_type type = CLIENT_COMMON);
    int UnSub_Rover_Client_Raw_Data(const char *client_key, const char *connect_key = "", Client_type type = CLIENT_COMMON);

    int Get_Source_Table_List(CasterCallback cb, void *arg, Source_type type = SOURCE_COMMON);
    int Add_Source_Table_Item(const char *mount_point, const char *info, double lon = 0.0, double lat = 0.0, Source_type type = SOURCE_COMMON);
    int Del_Source_Table_Item(const char *mount_point, Source_type type = SOURCE_COMMON);
    int Get_Source_Table_Item(const char *mount_point, CasterCallback cb, void *arg, Source_type type = SOURCE_COMMON);
    int Get_Radius_Table_List(double lon, double lat, CasterCallback cb, void *arg, Source_type type = SOURCE_COMMON);

    std::string Get_Source_Table_Text();

} // namespace CASTER

// Redis 内部维护表
// 一张
