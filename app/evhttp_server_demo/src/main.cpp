/*
    如何使用libevent搭建一个http服务器
*/

#include "http_server.h"

#include "event2/util.h"
#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <getopt.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else /* !_WIN32 */
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif /* _WIN32 */
#include <signal.h>

#ifdef EVENT__HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef EVENT__HAVE_AFUNIX_H
#include <afunix.h>
#endif

#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef _WIN32
#include <event2/thread.h>
#endif /* _WIN32 */

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif
#endif

#ifdef _WIN32
#ifndef stat
#define stat _stat
#endif
#ifndef fstat
#define fstat _fstat
#endif
#ifndef open
#define open _open
#endif
#ifndef close
#define close _close
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#endif /* _WIN32 */

int main()
{
    // 基础设置  设置监听端口
    int port = 4202;

    // // 可选项---------------------------------------
    // int iocp = 0; // WINDOWS 下生效  是否启动IOCP模型

    // const char *unixsock = nullptr;      // Unix Socket是一种在Unix/Linux操作系统下实现进程间通信（IPC）的一种方式
    // int unixsock_unlink_before_bind = 1; //    unlink unix socket before bind
    // //-----------------------------------------
#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        WSAStartup(wVersionRequested, &wsaData);
    }
#endif

    // 创建一个event_base
    event_base *base = event_base_new();
    if (!base)
    {
        fprintf(stderr, "Couldn't create an event_base: exiting\n");
        return 1;
    }

    // 创建一个evhttp
    evhttp *http = evhttp_new(base);
    if (!http)
    {
        fprintf(stderr, "couldn't create evhttp. Exiting.\n");
        return 1;
    }

    // 为不同的路径访问提供不同的回调函数

    evhttp_set_cb(http, "/test", test_request_cb, NULL);
    evhttp_set_cb(http, "/user", user_update_request_cb, NULL);
    evhttp_set_cb(http, "/mount", mount_update_request_cb, NULL);
    evhttp_set_cb(http, "/update", info_update_request_cb, NULL);

    // 设置通用回调函数，处理其他路径的请求
    evhttp_set_gencb(http, unset_request_cb, NULL);

    // evhttp绑定监听端口
    evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
    if (!handle)
    {
        fprintf(stderr, "couldn't bind to port %d. Exiting.\n", port);
        return 1;
    }

    // 启动event_base，监听事件
    event_base_dispatch(base);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
