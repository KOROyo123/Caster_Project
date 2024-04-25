#pragma once

#include "ntrip_global.h"

#include "Connector/ntrip_compat_listener.h"
#include "Connector/ntrip_relay_connector.h"
#include "Carrier/client_ntrip.h"
#include "Carrier/server_ntrip.h"
#include "Carrier/server_relay.h"
#include "Carrier/source_ntrip.h"
#include "DB/relay_account_tb.h"
#include "Extra/heart_beat.h"

#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <queue>
#include <list>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class ntrip_caster
{
private:
    json _service_setting;
    json _caster_core_setting;
    json _auth_verify_setting;

    json _common_setting;

    json _listener_setting;
    json _client_setting;
    json _server_setting;

    bool _output_state;
    int _timeout_intv;

public:
    // 公开的接口
    ntrip_caster(json cfg);
    ~ntrip_caster();

    int start();
    int stop();

private:
    // 状态数据
    json _state_info;
    int update_state_info();

    // 定期任务
    int periodic_task();

private:
    // 程序启动和停止
    int compontent_init();
    int compontent_stop();

private:
    // 任务处理函数
    int request_process(json req);

    int create_client_ntrip(json req); // 用Ntrip协议登录的用户(一个挂载点一个)
    int create_client_virtual(json req);
    int close_client_ntrip(json req);

    int create_server_ntrip(json req); // 基站主动接入产生的数据源
    int close_server_ntrip(json req);

    int create_source_ntrip(json req); // 用Ntrip协议获取源列表
    int close_source_ntrip(json req);  // 用Ntrip协议获取源列表

    // 请求处理失败，关闭连接
    int close_unsuccess_req_connect(json req);

private:
    // 连接器
    ntrip_compat_listener *_compat_listener;  // 被动接收Ntrip连接
    // ntrip_relay_connector *_relay_connetcotr; // 主动创建Ntrip连接

    // 连接-对象索引
    std::unordered_map<std::string, bufferevent *> _connect_map; // Connect_Key,bev或evhttp
    std::unordered_map<std::string, server_ntrip *> _server_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, client_ntrip *> _client_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, source_ntrip *> _source_map; // Connect_Key,client_ntrip

    // 挂载点-对象索引
    std::unordered_map<std::string, std::string> _server_key; // Mount_Point,Connect_Key

private:
    event_base *_base;
    // process处理事件
    event *_process_event;
    // 定时器和定时事件
    event *_timeout_ev;
    timeval _timeout_tv;

public:
    int start_server_thread();
    static void *event_base_thread(void *arg);

    // libevent回调
    static void Request_Process_Cb(evutil_socket_t fd, short what, void *arg);
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);

    static void Client_Check_Mount_Point_Callback(const char *request, void *arg, CatserReply *reply);
    static void Server_Check_Mount_Point_Callback(const char *request, void *arg, CatserReply *reply);
};
