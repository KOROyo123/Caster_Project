#pragma once

#include <event.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct heart_beat_setting
{
    bool PID;
    bool gpssecond;
    bool onlineServer;
    bool onlineClient;
    bool onlineTime;
    bool runningtime;
    bool memory;
};
struct heart_beat_info
{
    int PID;
    int gpssecond;
    int onlineServer;
    int onlineClient;
    int onlineTime;
    int runningtime;
    int memory;
};

enum heart_beat_method
{
    Pub,
    Set,
    HSet
};

class redis_heart_beat
{

private:
    json _heart_beat_config;

    bool _heart_beat_switch;

    // Redis连接相关
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

    // 定时事件
    event_base *_base;
    event *_timeout_ev;
    timeval _timeout_tv;

    // 输出相关
    heart_beat_method _heart_beat_method;
    std::string _heart_beat_key;
    json _heart_beat_set_msg;

    heart_beat_setting _msg_set;
    heart_beat_info _msg_info;

    time_t _start_tm;

    int _online_server;
    int _online_client;
    // std::string _module = "CDC_Plus";
    // std::string _version = "0.0.4";

public:
    redis_heart_beat(json conf);
    ~redis_heart_beat();

    int set_base(event_base *base);

    int start();
    int stop();

    int update_msg(json state_info);

public:
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);

    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

private:
    int connect_to_redis();
    
    int send_heart_beat();
    
    int refresh_beat_info();
    int update_out_set(json conf);

    json build_beat_msg();
    double getGnsssecond();
    int getMemory();
};
