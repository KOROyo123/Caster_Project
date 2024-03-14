/*
    全局生效的宏定义，宏定义应避免简单，写的详细一些
*/
#pragma once

// 工程配置
#include "version.h"
// #define SOFTWARE_NAME "KORO_Caster"
// #define SOFTWARE_VERSION "0.0.2"

// 指令类型与编号

// 开关
#define ENABLE_SYS_NTRIP_RELAY 101
#define DISABLE_SYS_NTRIP_RELAY 102
#define ENABLE_TRD_NTRIP_RELAY 103
#define DISABLE_TRD_NTRIP_RELAY 104
#define ENABLE_HTTP_SERVER 105
#define DISABLE_HTTP_SERVER 106


// 核心部件操作请求
#define CONNECT_TO_REDIS_SERVER 201
#define CREATE_NEW_NTRIP_LISTENER 202
#define CREATE_NEW_NTRIP_CONNECTOR 204
#define CREATE_DATA_TRANSFER 205
#define CREATE_NTRIP_SOURCE 207

// 一般ntrip请求
#define REQUEST_SOURCE_LOGIN 301
#define REQUEST_CLIENT_LOGIN 302
#define REQUEST_NEAREST_LOGIN 303
#define REQUEST_SERVER_LOGIN 305
#define CLOSE_NTRIP_CLIENT 306
#define CLOSE_NTRIP_SERVER 307

//连接操作请求
//#define CLOSE_NTRIP_SERVER 307

// relay服务相关
#define REQUEST_VIRTUAL_LOGIN 401
#define CREATE_RELAY_SERVER 402
#define CLOSE_RELAY_SERVER 403
#define ADD_RELAY_MOUNT_TO_LISTENER 404
#define CLOSE_RELAY_REQ_CONNECT 405
#define ADD_RELAY_MOUNT_TO_SOURCELIST 406

// 验证相关
#define MOUNT_NOT_ONLINE_CLOSE_CONNCET 501
#define MOUNT_ALREADY_ONLINE_CLOSE_CONNCET 502
#define NO_IDEL_RELAY_ACCOUNT_CLOSE_CONNCET 503
#define CREATE_RELAY_CONNECT_FAIL_CLOSE_CONNCET 504
#define ALREADY_SEND_SOURCELIST_CLOSE_CONNCET 505

#define CARRIER_TYPE_EVHTTP_REQUEST 1
#define CARRIER_TYPE_BUFFEREVENT 2

// req item
#define ITEM_REQ_TYPE "req_type"
#define ITEM_ORIGIN_REQ "origin_req"
#define ITEM_CARRIER_TYPE "carrier_type"
#define ITEM_CONNECT_KEY "connect_key"

#define ITEM_MOUNT_POINT "mount_point"
#define ITEM_MOUNT_GROUP "user_group"
#define ITEM_MOUNT_VERSION "ntrip_version"

#define ITEM_USER_NAME "user"
#define ITEM_USER_PWD "pwd"
#define ITEM_USER_NAME_PWD "usr_pwd"
#define ITEM_USER_BASE64ID "userID"

// enum SERVER_REQUEST_REQ
// {
//     CREATE_NEW_NTRIP_LISTENER,
//     CONNECT_TO_REDIS_SERVER,
//     REQUEST_SOURCE_LIST,
//     REQUEST_CLIENT_LOGIN,
//     REQUEST_SERVER_LOGIN,
//     REQUEST_NEAREST_LOGIN,
//     REQUEST_VIRTUAL_LOGIN,
//     CLOSE_NTRIP_SERVER,
//     CLOSE_NTRIP_CLIENT,
//     CREATE_DATA_TRANSFER,
//     CLOSE_DATA_TRANSFER,
//     CREATE_VIRTUAL_SERVER,
//     CLOSE_VIRTUAL_SERVER,
//     TRANS_SERVER_DATA
// };

// enum CONNECTION_MODIFY_PROCESS
// {
//     RESULT_CLOSE_CONNCET
// }

// 数据传输
// 连接类型
#define TCP_CONNECT_TYPE_COMMON_NTRIP 01  // 被动连接类型    通过NTRIP_ACCEPT监听的端口连入的连接  适用于 client sever  relayclient
#define TCP_CONNECT_TYPE_ACTIVE_NTRIP 02  // 主动连接类型    通过NTRIP_CONNECT主动连接的端口
#define TCP_CONNECT_TYPE_COMMON_DIRECT 03 // TCP直连 主动    通过TCP_CONNECT主动连接的端口
#define TCP_CONNECT_TYPE_ACTIVE_DIRECT 04 // TCP直连 被动    通过TCP_ACCEPT监听的端口连入的连接

// 用户类型
#define TCP_CONNECT_TYPE_CLIENT_NTRIP 10   // Ntrip Client类型连接（挂载点访问模式）   【已支持】
#define TCP_CONNECT_TYPE_CLIENT_RELAY 11   // Ntrip Client类型连接（第三方服务模式）   【未支持】
#define TCP_CONNECT_TYPE_CLIENT_NEAREST 11 // Ntrip Client类型连接（最近点连接模式）   【未支持】
#define TCP_CONNECT_TYPE_VIRTAL_VIRTAL 12  // Ntrip Client类型连接（虚拟参考站模式)
#define TCP_CONNECT_TYPE_GET_LIST 13       // Ntrip Client类型连接（获取挂载点列表）   【已支持】
// 服务类型
#define TCP_CONNECT_TYPE_SERVER_NTRIP 20  // Ntrip Server类型连接(被动连接的挂载点）  【已支持】
#define TCP_CONNECT_TYPE_SERVER_RELAY 21  // Ntrip Server类型连接(主动连接的挂载点）  【未支持】
#define TCP_CONNECT_TYPE_SERVER_VIRTAL 22 // Ntrip Server类型连接(虚拟产生的挂载点）
// 直连类型
#define TCP_CONNECT_TYPE_TCPSVR_RELAY 30 // TCP Server类型连接（主动连接TCP数据流）
#define TCP_CONNECT_TYPE_TCPCLI_RELAY 31 // TCP Server类型连接（主动连接TCP数据流）
// 交互与控制
#define TCP_CONNECT_TYPE_QUEUE_GUI_CLIENT 40 // 图形化用户程序连接（用于图形化桌面程序）  【开发中】
#define TCP_CONNECT_TYPE_QUEUE_CUI_CLIENT 41 // 控制台用户程序连接（用于控制台指令程序）
#define TCP_CONNECT_TYPE_QUEUE_MSG_QUEUES 42 // 消息队列Broker连接（用于网页程序）

// enum SERVER_REQUEST_REQ
// {
//     TCP_CONNECT_TYPE_COMMON_NTRIP,
//     TCP_CONNECT_TYPE_ACTIVE_NTRIP,
//     TCP_CONNECT_TYPE_COMMON_DIRECT,
//     TCP_CONNECT_TYPE_ACTIVE_DIRECT,
//     TCP_CONNECT_TYPE_CLIENT_NTRIP,
//     TCP_CONNECT_TYPE_CLIENT_RELAY,
//     TCP_CONNECT_TYPE_CLIENT_NEAREST,
//     TCP_CONNECT_TYPE_VIRTAL_VIRTAL,
//     TCP_CONNECT_TYPE_GET_LIST,
//     TCP_CONNECT_TYPE_SERVER_NTRIP,
//     TCP_CONNECT_TYPE_SERVER_RELAY,
//     TCP_CONNECT_TYPE_SERVER_VIRTAL,
//     TCP_CONNECT_TYPE_TCPSVR_RELAY,
//     TCP_CONNECT_TYPE_TCPCLI_RELAY,
//     TCP_CONNECT_TYPE_QUEUE_GUI_CLIENT,
//     TCP_CONNECT_TYPE_QUEUE_CUI_CLIENT,
//     TCP_CONNECT_TYPE_QUEUE_MSG_QUEUES
// }

// 时间
#define EVENT_TIMEOUT_SEC 5

// 大小相关宏定义
#define VERIFY_INFO_H_MAX_CHAR 128

#define BUFFEVENT_READ_DATA_SIZE 4096

#define MAX_CAHR 128

// 验证语句相关宏定义

// 请求类型的宏定义

// 请求：处理一个新的普通被动连接
#define REQUEST_NEW_CONNECT_ALLOC 1

// 请求：连接第三方挂载点
#define REQUEST_THIRD_PART_SERVER_LINK 2
// 请求：第三方挂载点连接成功，处理该连接
#define REQUEST_THIRD_PART_SERVER_ALLOC 3

// rtklib的

#define TINTACT 200              /* period for stream active (ms) */
#define SERIBUFFSIZE 4096        /* serial buffer size (bytes) */
#define TIMETAGH_LEN 64          /* time tag file header length */
#define MAXCLI 32                /* max client connection for tcp svr */
#define MAXSTATMSG 32            /* max length of status message */
#define DEFAULT_MEMBUF_SIZE 4096 /* default memory buffer size (bytes) */

#define NTRIP_AGENT "RTKLIB/" VER_RTKLIB
#define NTRIP_CLI_PORT 2101                       /* default ntrip-client connection port */
#define NTRIP_SVR_PORT 80                         /* default ntrip-server connection port */
#define NTRIP_MAXRSP 32768                        /* max size of ntrip response */
#define NTRIP_MAXSTR 256                          /* max length of mountpoint string */
#define NTRIP_RSP_OK_CLI "ICY 200 OK\r\n"         /* ntrip response: client */
#define NTRIP_RSP_OK_SVR "OK\r\n"                 /* ntrip response: server */
#define NTRIP_RSP_SRCTBL "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
#define NTRIP_RSP_TBLEND "ENDSOURCETABLE"
#define NTRIP_RSP_HTTP "HTTP/"  /* ntrip response: http */
#define NTRIP_RSP_ERROR "ERROR" /* ntrip response: error */
#define NTRIP_RSP_UNAUTH "HTTP/1.0 401 Unauthorized\r\n"
#define NTRIP_RSP_ERR_PWD "ERROR - Bad Pasword\r\n"
#define NTRIP_RSP_ERR_MNTP "ERROR - Bad Mountpoint\r\n"

// 所有需要修改表的操作，都采用通知机制？

// 连接表
// 请求：新增连接

// 请求：断开连接（主动断开）

// 通知：连接断开（被动断开，或触发了断开机制）

// 基站

// 请求：挂载点上线
// 请求：挂载点下线（主动断开）
// 通知：挂载点下线（同时要关闭连接）

// 请求：更新挂载点信息
#define NOTICE_

// 用户

// 通知：移动站上线

// 通知：移动站下线

// 请求：订阅指定挂载点

// 请求：更改订阅挂载点


