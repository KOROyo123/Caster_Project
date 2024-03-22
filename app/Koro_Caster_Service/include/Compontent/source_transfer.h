#pragma once

#include "process_queue.h"

#include <unordered_map>
#include <set>

#include "ntrip_global.h"
#include "source_ntrip.h"

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <spdlog/spdlog.h>

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

class source_transfer
{
private:
    json _setting;

    std::shared_ptr<process_queue> _queue;

    // 加一个定时器事件，定时获取所有在线挂载点
    event_base *_base;
    event *_source_update_ev;
    timeval _source_update_tv;
    // event *_delay_close_ev;
    // timeval _delay_close_tv;
    // 源列表输出选项
    bool _send_common;    // 普通挂载点
    bool _send_trd_relay; // 用户连接的第三方挂载点
    bool _send_sys_relay; // 系统转发的挂载点
    bool _send_nearest;   // NEAREST是否可见//Nearest
    bool _send_virtual;   // Virtual是否可见//设置的挂载点

    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

    std::unordered_map<std::string, mount_info> _mount_map; // Mount_Point // 挂载点名为XXXX-F1A6(虚拟挂载点名-本地连接第三方时采用的端口转为4位16进制)

    std::set<std::string> _online_common_mount;
    std::set<std::string> _online_sys_relay_mount;
    std::set<std::string> _online_trd_relay_mount;
    std::set<std::string> _support_virtual_mount;
    std::set<std::string> _support_nearest_mount;

    std::string _common_items;
    std::string _sys_relay_items;
    std::string _trd_relay_items;
    std::string _virtual_items;
    std::string _nearest_items;

    std::string _all_items;

    // evbuffer* _ntrip1_source_table=evbuffer_new();
    // evbuffer* _ntrip2_source_table=evbuffer_new();
    std::string _ntrip1_source_table;
    std::string _ntrip2_source_table;

    // std::list<json> _delay_close_list[2];
    // std::list<json> *_using_delay_close_list;

public:
    source_transfer(json req, event_base *base, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context);
    ~source_transfer();

    int start();
    int stop();

    // int send_source_list_to_client(json req, void *connect_obj);

    std::string get_souce_list();

    int add_Virtal_Mount(std::string mount_point);
    int del_Virtal_Mount(std::string mount_point);

private:
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);
    //static void DelayCloseCallback(evutil_socket_t fd, short events, void *arg);


    static void Redis_Callback_Get_Common_List(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback_Get_SYS_Relay_List(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback_Get_TRD_Relay_List(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback_Get_All_List(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback_Get_One_info(redisAsyncContext *c, void *r, void *privdata);

private:
    int get_online_mount_point();

    int build_source_list();
    int build_source_table();

    int close_delay_close();

    std::string update_list_item(std::set<std::string> group);

    std::string build_mount_info_to_string(mount_info item);
    mount_info build_default_mount_info(std::string mount_point);
};

// 定期拉取所有在线挂载点信息

// 根据传入的对象，检索所在分组

// 发送挂载点信息

// 不同的挂载点列表由不同的source对象维护

// 新连接来了传入，解析需求，发送，关闭对应连接