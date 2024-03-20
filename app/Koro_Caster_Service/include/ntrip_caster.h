#pragma once

#include "ntrip_global.h"

#include "Connector/ntrip_compat_listener.h"
#include "Connector/ntrip_relay_connector.h"
#include "Carrier/client_ntrip.h"
#include "Carrier/server_ntrip.h"
#include "Carrier/server_relay.h"
#include "Compontent/client_source.h"
#include "Compontent/data_transfer.h"
#include "Compontent/auth_verifier.h"
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
    json _server_config;

    // 一些开关
    bool _Server_Login_With_Password = false;
    bool _Client_Login_With_Password = false;

    bool _SYS_Relay_Support = false;
    bool _TRD_Relay_Support = false;
    bool _HTTP_Ctrl_Support = false;
    bool _Extra_Module_Support = false;

    // Redis连接相关
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

public:
    // 公开的接口
    ntrip_caster(json cfg);
    ~ntrip_caster();

    int start();
    int stop();

    json get_setting();
    int set_setting(json config);

private:
    // 状态数据
    json _state_info;
    int init_state_info();
    int update_state_info();

    // 定期任务
    int periodic_task();

private:
    // 程序启动和停止
    int auto_init();
    int compontent_init();
    int extra_init();

    int auto_stop();
    int compontent_stop();
    int extra_stop();

private:
    // 任务处理函数
    int request_process(json req);

    // redis相关
    int create_redis_conncet(json req);
    int destroy_redis_conncet(json req);
    int reconnect_redis_connect(json req);

    // data_transfer 相关
    int create_data_transfer(json req);
    int destroy_data_transfer(json req);

    // ntrip common 相关
    int create_ntrip_listener(json req);
    int destroy_ntrip_listener(json req);
    int create_client_source(json req);
    int destroy_client_source(json req);

    int create_client_ntrip(json req); // 用Ntrip协议登录的用户(一个挂载点一个)
    int close_client_ntrip(json req);
    int create_server_ntrip(json req); // 基站主动接入产生的数据源
    int close_server_ntrip(json req);
    int create_client_nearest(json req);
    int send_souce_list(json req); // 用Ntrip协议获取源列表

    // ntrip relay 相关
    int create_relay_connector(json req);
    int create_relay_connect(json req);
    int close_realy_req_connection(json req);
    int create_server_relay(json req); // 主动连接其他caster的数据源
    int close_server_relay(json req);
    int transfer_add_create_client(json req); // 用户请求第三方挂载点上线后，添加到trransfer并创建client

    // 异常连接管理
    int mount_not_online_close_connect(json req);
    int close_unsuccess_req_connect(json req);

    // 添加支持的虚拟挂载点
    int add_relay_mount_to_listener(json req);
    int add_relay_mount_to_sourcelist(json req);

private:
    // Extra功能
    // Redis心跳
    redis_heart_beat *_redis_beat;
    //
private:
    // 连接器
    ntrip_compat_listener *_compat_listener;  // 被动接收Ntrip连接
    ntrip_relay_connector *_relay_connetcotr; // 主动创建Ntrip连接

    // 核心组件
    data_transfer *_transfer;   // 数据转发
    client_source *_sourcelist; // 挂载点列表
    auth_verifier *_verifier;   // 密码验证

    // 连接-对象索引
    std::unordered_map<std::string, bufferevent *> _connect_map; // Connect_Key,bev或evhttp
    std::unordered_map<std::string, server_ntrip *> _server_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, client_ntrip *> _client_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, server_relay *> _relays_map; // Connect_Key,server_relay // 挂载点名为XXXX-F1A6(虚拟挂载点名-本地连接第三方时采用的端口转为4位16进制)
    
    //挂载点-对象索引
    std::unordered_map<std::string, std::string> _server_key; // Mount_Point,Connect_Key
    std::unordered_map<std::string, std::string> _relays_key; // Mount_Point,Connect_Key

    // relay账号表
    relay_account_tb _relay_accounts;

private:
    // 任务队列
    std::shared_ptr<process_queue> _queue = std::make_shared<process_queue>();
    // base
    event_base *_base;
    // process处理事件
    event *_process_event;
    // 定时器和定时事件
    event *_timeout_ev;
    timeval _timeout_tv;

    timeval _delay_exit_tv = {1, 0}; // 延迟关闭定时器

    //redis发布订阅连接
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    int start_server_thread();
    static void *event_base_thread(void *arg);

    // libevent回调
    static void Request_Process_Cb(evutil_socket_t fd, short what, void *arg);
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);

    // Redis回调
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

    static void Redis_Callback_for_Data_Transfer_add_sub(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_Callback_for_Create_Ntrip_Server(redisAsyncContext *c, void *r, void *privdata);
};
