#pragma once
#include <string>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

struct server_info_item
{
    std::string mount_point; // 挂载点
    std::string data_type;   // 数据类型
    double blh[3];           // 坐标
    int rover_num;           // 移动站数量
    std::string online_time; // 上线时刻
    time_t online_seconds;   // 在线时长
    // tcp
    std::string ip;
    int port;
    int receive_speed;  // 接收数据速度 b/s
    long receive_total; // 接收数据量   b
};

struct client_info_item
{
    std::string user_name;   // 用户名
    std::string mount_point; // 挂载点
    std::string use_station; // 实际使用的基站
    double blh[3];           // 坐标
    double speed;            // 速度
    int gpsStatus;           // 定位状态
    int usedSatNum;          // 使用卫星数
    double diffTime;         // 差分延迟
    double distance;         // 距离基站位置
    double base_blh[3];      // 基站坐标
    int gpsTime_year, gpsTime_mouth, gpsTime_day, gpsTime_hour, gpsTime_min;
    double gpsTime_sec;      // GPS时间
    std::string online_time; // 上线时刻
    time_t online_seconds;   // 在线时长
    // tcp
    std::string ip;
    int port;
    int send_speed;  // 发送数据速度  b/s
    long send_total; // 发送数据量   b
};

class redis_info_push
{
private:
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    event_base *_base;
    redisAsyncContext *_context;

public:
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_info_push(/* args */);
    ~redis_info_push();

    int Init(const char *confpath);
    int Start();
    int Stop();

    static void *event_base_thread(void *arg);

    int push_base_info(server_info_item item);
    int push_rover_info(client_info_item item);

    std::string parse_base_to_value(server_info_item &item);
    std::string parse_rover_to_value(client_info_item &item);
    long long get_time_stamp();
};
