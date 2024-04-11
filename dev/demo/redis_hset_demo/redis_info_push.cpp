#include "redis_info_push.h"

#include <chrono>

#include <event2/event.h>
#include <event2/thread.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

redis_info_push::redis_info_push(/* args */)
{
}

redis_info_push::~redis_info_push()
{
}

int redis_info_push::Init(const char *confpath)
{
    _redis_IP = "127.0.0.1";
    _redis_port = 16379;
    _redis_Requirepass = "koroyo123";

    _base = event_base_new();

    return 0;
}

int redis_info_push::Start()
{
    _context = redisAsyncConnect(_redis_IP.c_str(), _redis_port);
    if (_context->err)
    {
        return -1;
    }
    redisLibeventAttach(_context, _base);
    redisAsyncSetConnectCallback(_context, NULL);
    redisAsyncSetDisconnectCallback(_context, NULL);
    // redisAsyncCommand(_sub_context, Redis_Sub_Check_Callback, this, "AUTH %s", _redis_Requirepass.c_str());
    redisAsyncCommand(_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

#ifdef WIN32
    evthread_use_windows_threads(); // libevent启用windows线程函数
#else
    evthread_use_pthreads(); // libenvet启用linux线程函数
#endif

    pthread_t id;
    int ret1 = pthread_create(&id, NULL, event_base_thread, _base);
    if (ret1)
    {
        return -1;
    }

    return 0;
}

int redis_info_push::Stop()
{
    return 0;
}

void *redis_info_push::event_base_thread(void *arg)
{
    event_base *base = (event_base *)arg;

    evthread_make_base_notifiable(base);
    // event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件
    event_base_dispatch(base);

    return nullptr;
}

int redis_info_push::push_base_info(server_info_item item)
{
    return 0;
}

int redis_info_push::push_rover_info(client_info_item item)
{

    return 0;
}

std::string redis_info_push::parse_base_to_value(server_info_item &item)
{
    json info;

    info["mount_point"] = item.mount_point;
    info["data_type"] = item.data_type;
    info["coord_b"] = item.blh[0];
    info["coord_l"] = item.blh[1];
    info["coord_h"] = item.blh[2];

    info["rover_num"] = item.rover_num;
    info["online_time"] = item.online_time;
    info["online_seconds"] = item.online_seconds;

    info["ip"] = item.ip;
    info["port"] = item.port;

    info["receive_speed"] = item.receive_speed;
    info["receice_total"] = item.receive_total;

    info["update_time"] = get_time_stamp();

    std::string value = info.dump();

    return value;
}

std::string redis_info_push::parse_rover_to_value(client_info_item &item)
{
    json info;

    info["user_name"] = item.user_name;
    info["mount_point"] = item.mount_point;
    info["use_station"] = item.use_station;
    info["coord_b"] = item.blh[0];
    info["coord_l"] = item.blh[1];
    info["coord_h"] = item.blh[2];
    info["speed"] = item.speed;

    info["gpsStatus"] = item.gpsStatus;
    info["usedSatNum"] = item.usedSatNum;
    info["diffTime"] = item.diffTime;
    info["distance"] = item.distance;
    info["base_b"] = item.base_blh[0];
    info["base_l"] = item.base_blh[1];
    info["base_h"] = item.base_blh[2];

    char gpstime[100] = "\0";
    sprintf(gpstime, "%d-%d-%d %d:%d:%lf", item.gpsTime_year, item.gpsTime_mouth, item.gpsTime_day, item.gpsTime_hour, item.gpsTime_min, item.gpsTime_sec);
    info["gpstime"] = gpstime;

    info["online_time"] = item.online_time;
    info["online_seconds"] = item.online_seconds;

    info["ip"] = item.ip;
    info["port"] = item.port;

    info["send_speed"] = item.send_speed;
    info["send_total"] = item.send_total;

    info["update_time"] = get_time_stamp();

    std::string value = info.dump();

    return value;
}

long long redis_info_push::get_time_stamp()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // 转换为时间类型
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 获取秒数
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    long long seconds_count = seconds.count();
    return seconds_count;
}
