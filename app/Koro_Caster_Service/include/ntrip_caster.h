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

private:
    // Extra功能
    // Redis心跳
    redis_heart_beat *_redis_beat;
    //
private:
    int auto_init();
    int task_init();
    int extra_init();

    // 定期任务
    int periodic_task();

private:
    // 连接处理函数
    int request_process(json req);

    // redis相关
    int redis_conncet_to(json req);
    int redis_reconnect_to(json req);

    // data_transfer 相关
    int create_data_transfer(json req);
    int transfer_add_create_client(json req); // 用户请求第三方挂载点上线后，添加到trransfer并创建client

    // ntrip common 相关
    int create_ntrip_listener(json req);
    int create_client_source(json req);
    int create_client_ntrip(json req); // 用Ntrip协议登录的用户(一个挂载点一个)
    int close_client_ntrip(json req);
    int create_server_ntrip(json req); // 基站主动接入产生的数据源
    int close_server_ntrip(json req);

    // ntrip relay 相关
    int create_relay_connector(json req);
    int create_relay_connect(json req);
    int close_realy_req_connection(json req);
    int create_server_relay(json req); // 主动连接其他caster的数据源
    int close_server_relay(json req);

    // 施工中----------------------------------------------------------------------------------------------
    int tcpsvr_listen_to(json req);  // bufferevent   根据请求端口，构建类型  传入参数（对于server_tcpsvr
    int tcpsvr_connect_to(json req); // bufferevent   根据请求类型，构建类型  传入参数（对于server_tcpcli

    int send_souce_list(json req);      // 用Ntrip协议获取源列表(一个用户一个)
    int create_client_tcpcli(json req); // 用TCP协议获取数据的用户（一个端口一个）
    int create_client_nearest(json req);

    int create_server_tcpcli(json req); // 主动连接其他tcpsvr产生的数据源
    int create_server_tcpsvr(json req); // TCP主动接入产生的数据源

    // 施工中----------------------------------------------------------------------------------------------

    // 异常连接管理
    int mount_not_online_close_connect(json req);
    int close_unsuccess_req_connect(json req);

    // 添加支持的虚拟挂载点
    int add_relay_mount_to_listener(json req);
    int add_relay_mount_to_sourcelist(json req);

    // 连接控制器
private:
    // 监听器map
    ntrip_compat_listener *_compat_listener;  // 被动接收Ntrip连接
    ntrip_relay_connector *_relay_connetcotr; // 主动创建Ntrip连接

    data_transfer *_transfer;   // 数据转发
    client_source *_sourcelist; // 挂载点列表
    auth_verifier *_verifier;   // 密码验证

    // 连接器map
    std::unordered_map<std::string, bufferevent *> _connect_map; // Connect_Key,bev或evhttp
    std::unordered_map<std::string, server_ntrip *> _server_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, client_ntrip *> _client_map; // Connect_Key,client_ntrip
    std::unordered_map<std::string, server_relay *> _relays_map; // Connect_Key,server_relay // 挂载点名为XXXX-F1A6(虚拟挂载点名-本地连接第三方时采用的端口转为4位16进制)

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

    // Redis连接相关
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;
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
