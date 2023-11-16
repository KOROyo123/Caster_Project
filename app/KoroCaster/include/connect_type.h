#pragma once

#include <queue>
#include <condition_variable>
#include <string>
#include "event.h"

#include <event2/util.h>
#include <nlohmann/json.hpp>

#include "ntrip_global.h"

#include "DB/tcp_connect_table.h"
#include "DB/ntrip_client_table.h"
#include "DB/ntrip_server_table.h"
#include "DB/ntrip_subclient_table.h"
#include "DB/relay_account_table.h"

using json = nlohmann::json;

// 类声明（因为牵扯到互相包含，所以要提前声明）
class tcp_connect;
class ntrip_server;
class ntrip_client;
class relay_server;
class relay_client;
class que_client;

class ntripcenter_list
{
public:
    tcp_connect_table tcp_list;
    ntrip_client_table client_list;
    ntrip_server_table server_list;
    ntrip_subclient_table subclient_list;
    relay_account_table relay_account_list;
};

struct connect_info_t
{
    /* data */
    int type;
    evutil_socket_t fd;
    int ntrip2; // 是否采用Ntrip2.0协议
    char http_version[64];

    char Expiration_time[64]; //

    char ConMount[64];   // 接入的挂载点，连接的挂载点(名义上的挂载点)
    char LocalMount[64]; // 本地映射的挂载点，存在表中的挂载点，一切以这个为准（实际上使用的挂载点）

    char UserName[64];
    char Password[64];
    char UserName_pwd[64];
    char Base64UserID[64];

    char addr[MAX_CAHR];
    char host[MAX_CAHR];
    int port;

    int Proactive_close_connect_flag; // 主动关闭标记（只是用来判断，该连接是主动关闭的还是被动关闭的）

    // 其他信息
    // 采用的软件及版本等
    char User_Agent[64];
};
struct relay_info_t
{
    char host[64];
    int port;
    char Mount[64];
    char base64[64];

    char LocalMount[64];
};

/*
    tcp connect类，作为所有连接的基类：
    连接创建的时候，会创建一个该类对象，
    提供验证连接的功能，验证成功后会对连接进行进一步操作，不成功则会断开连接
    根据数据验证连接类型，创建一个子类（由子类来实现具体的功能），并分配到指定的表中
    所有的tcp connect类都统一由一个表来记录，用于管理所有tcp连接
*/
class tcp_connect
{
public:
    // 被动连接的构造函数
    tcp_connect(event_base *svr_base, ntripcenter_list *list, bufferevent *base_bev, int connect_type, evutil_socket_t conn_fd);

    // 主动连接的构造函数
    tcp_connect(event_base *svr_base, ntripcenter_list *list, bufferevent *base_bev, int connect_type, evutil_socket_t conn_fd, json *relayinfo);

    tcp_connect();
    ~tcp_connect();

    // 数据接发
    int start(); // 绑定readcb，开始接收数据并处理
    void stop(); // 真正的关闭

    int delay_close(int millisecond); // 关闭套接字，触发eventcallback

private:
    int login_verify(evbuffer *recv_data);

    int decode_header(evbuffer *recv_data);

    int decode_get_request(char *request);
    int decode_source_request(char *request);
    int decode_post_request(char *request);

    int ntrip_client_header_decode(evbuffer *recv_data);
    int ntrip_server_header_decode(evbuffer *recv_data);
    int ntrip_relay_header_decode(int use_ntrip2);

    int check_userID(); // 检测userID是否有效，并更新有效期（如果有效期更新了，那就更新有效期）

    int check_mountPoint();

    int match_relay_mount(); // 查询是否存在第三方挂载点

    int create_connect_by_type(); // 验证成功

    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void WriteCallback(bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);
    static void LicenceExpireCallback(evutil_socket_t fd, short events, void *arg);

    static void DelayCloseCallback(evutil_socket_t fd, short what, void *arg);

    // 表维护
    int add_to_table(); // 连接建立，将该类添加到表中（是构造后（初始化完）应当执行的第一个工作）
    int del_to_table(); // 连接断开，将该类从表中移除（是析构前的（连接清楚）应当执行的最后一个工作）

private:
    int init();

    int send_relay_request();

private:
    bufferevent *_bev;

    timeval _bev_read_timeout, bev_write_timeout;

    event *_timeev;
    timeval _timeout;

    connect_info_t *_connect_info;

private:
    event_base *_svr_base;
    ntripcenter_list *_list;
    bufferevent *_base_bev; // 给ntripcenter发送指令的通道

    int _error_type;
    ntrip_server *_ntrip_server = nullptr;
    ntrip_client *_ntrip_client = nullptr;
    relay_server *_relay_server = nullptr;
    relay_client *_relay_client = nullptr;
    que_client *_que_client = nullptr;
};

/*
    ntrip server类：
    当TCP验证为Ntrip Server类型，并且账号密码有效期等验证通过后会创建一个该对象

    所有同一类型/子网的ntrip server连接会由一个表记录，以进行分类，并控制用户对其的可见性，
    不同类型的ntrip server（例如不同框架，不同用户组，可以用的挂载点都不同）
*/
class ntrip_server
{
public:
    ntrip_server();
    ntrip_server(event_base *base, ntripcenter_list *list, connect_info_t *connect_info);
    ~ntrip_server();

    int init();

    static void EventCallback(struct bufferevent *bev, short events, void *arg);
    static void ReadCallback(struct bufferevent *bev, void *arg);

    int start();
    int stop();

    int set_bev_timeout(int read_timeout_sec, int write_timeout_sec);

    int add_to_table();
    int del_to_table();

    int data_dispatch(evbuffer *recv_data);
    int data_decode(evbuffer *recv_data);

private:
    bufferevent *_bev;

    event *_timeev;
    timeval _timeout;

    timeval *_tv_read;
    timeval *_tv_write;

    connect_info_t *_connect_info;

private:
    event_base *_svr_base;
    ntripcenter_list *_list;
    bufferevent *_base_bev; // 给ntripcenter发送指令的通道
};

/*
    ntrip client类
    当TCP验证为Ntrip client类型，并且账号密码有效期等验证通过后会创建一个该对象

    所有同一类型/子网的ntrip client连接会由一个表记录，以进行分类，并控制其对指定挂载点表的访问，
    不同类型的ntrip client（例如不同框架，不同用户组，可以用的挂载点都不同）

*/
class ntrip_client
{
public:
    ntrip_client(event_base *svr_base, ntripcenter_list *list, connect_info_t *connect_info);
    ntrip_client();
    ~ntrip_client();

    int start();
    int stop();

    int delay_close(int millisecond);

    int link_connect();
    int close_connect();

private:
    int add_to_table();
    int del_to_table();

    int find_and_subscribe(); // 找到匹配的挂载点，把自己加入到其中
    int dis_subcribe();       // 移除现有的订阅

    int find_and_update_sub_mount(); // 根据连接类型找合适的挂载点
    int add_to_subclient_table();
    int del_from_subclient_table();

public:
    static void ReadCallback(struct bufferevent *bev, void *arg);
    static void EventCallback(struct bufferevent *bev, short events, void *arg);

    static void TimeoutCallback(evutil_socket_t *bev, short events, void *arg);
    static void LicenceCheckCallback(evutil_socket_t *bev, short events, void *arg);

private:
    bufferevent *_bev;

    event *_timeev;
    timeval _timeout;

    connect_info_t *_connect_info;

private:
    event_base *_svr_base;
    ntripcenter_list *_list;
    bufferevent *_base_bev; // 给ntripcenter发送指令的通道
};

class relay_server
{
public:
    int recvGGA();

private:
    double VertualGGA[3];
};

class relay_client
{
public:
    relay_client();

    // static void ReadCallback(struct bufferevent *bev, void *arg);

    int sentGGA();
};

/*
    gui client类

*/
class que_client
{
public:
    que_client();
    int start();
};
