#pragma once
#include <string>

#include "event2/event.h"

class http_heartbeat
{
private:
    event_base *_base;
    std::string _url;
    std::string _module = "CDC_Plus";
    std::string _IP;
    int _PID;
    int _gnssecond;

    time_t _start_tm;

    int _onlineServer;
    int _onlineclient;
    int _onlineTime;

    std::string _heart_beat_msg;

public:
    http_heartbeat(/* args */);
    ~http_heartbeat();

    int set_event_base(event_base *base);
    int set_url(const char *url);

    int update_info(int Server_count, int Client_count);

    int send_heartbeat();

    static void http_heartbeat_callback(struct evhttp_request *req, void *arg);

private:
    int get_sys_info();
    int build_heartbeat_msg();
};