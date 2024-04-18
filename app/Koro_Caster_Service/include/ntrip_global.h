/*
    全局生效的宏定义，宏定义应避免简单，写的详细一些
*/
#pragma once

// 工程配置
#include "version.h"
#include "Caster_Core.h"
#include "Auth_Verify.h"
// #define SOFTWARE_NAME "KORO_Caster"
// #define SOFTWARE_VERSION "0.0.2"

//挂载点类型

#define MOUNT_TYPE_COMMON 1
#define MOUNT_TYPE_NEAREST 2
#define MOUNT_TYPE_RELAY 3
#define MOUNT_TYPE_VIRTUAL 4





// 开关
#define ENABLE_SYS_NTRIP_RELAY 101
#define DISABLE_SYS_NTRIP_RELAY 102
#define ENABLE_TRD_NTRIP_RELAY 103
#define DISABLE_TRD_NTRIP_RELAY 104
#define ENABLE_HTTP_SERVER 105
#define DISABLE_HTTP_SERVER 106

// 核心部件操作请求

// 一般ntrip请求
#define REQUEST_SOURCE_LOGIN 301
#define CLOSE_NTRIP_SOURCE 302

#define REQUEST_CLIENT_LOGIN 303
#define REQUEST_NEAREST_LOGIN 304
#define REQUEST_RELAY_LOGIN 305
#define REQUEST_VIRTUAL_LOGIN 306
#define CLOSE_NTRIP_CLIENT 307

#define REQUEST_SERVER_LOGIN 308
#define CLOSE_NTRIP_SERVER 309

// 连接操作请求
// #define CLOSE_NTRIP_SERVER 307

// relay服务相关
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

// // 时间
// #define EVENT_TIMEOUT_SEC 5

// // 大小相关宏定义
// #define VERIFY_INFO_H_MAX_CHAR 128

// #define BUFFEVENT_READ_DATA_SIZE 4096

// #define MAX_CAHR 128

// rtklib的

// #define TINTACT 200              /* period for stream active (ms) */
// #define SERIBUFFSIZE 4096        /* serial buffer size (bytes) */
// #define TIMETAGH_LEN 64          /* time tag file header length */
// #define MAXCLI 32                /* max client connection for tcp svr */
// #define MAXSTATMSG 32            /* max length of status message */
// #define DEFAULT_MEMBUF_SIZE 4096 /* default memory buffer size (bytes) */

// #define NTRIP_AGENT "RTKLIB/" VER_RTKLIB
// #define NTRIP_CLI_PORT 2101                       /* default ntrip-client connection port */
// #define NTRIP_SVR_PORT 80                         /* default ntrip-server connection port */
// #define NTRIP_MAXRSP 32768                        /* max size of ntrip response */
// #define NTRIP_MAXSTR 256                          /* max length of mountpoint string */
// #define NTRIP_RSP_OK_CLI "ICY 200 OK\r\n"         /* ntrip response: client */
// #define NTRIP_RSP_OK_SVR "OK\r\n"                 /* ntrip response: server */
// #define NTRIP_RSP_SRCTBL "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
// #define NTRIP_RSP_TBLEND "ENDSOURCETABLE"
// #define NTRIP_RSP_HTTP "HTTP/"  /* ntrip response: http */
// #define NTRIP_RSP_ERROR "ERROR" /* ntrip response: error */
// #define NTRIP_RSP_UNAUTH "HTTP/1.0 401 Unauthorized\r\n"
// #define NTRIP_RSP_ERR_PWD "ERROR - Bad Pasword\r\n"
// #define NTRIP_RSP_ERR_MNTP "ERROR - Bad Mountpoint\r\n"
