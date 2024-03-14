#include "http_heart_beat.h"

#include "nlohmann/json.hpp"

#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include <unistd.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rtklib.h"

using json = nlohmann::json;

int http_heartbeat::send_heartbeat()
{

    evhttp_uri *uri = evhttp_uri_parse(_url.c_str());

    const char *scheme = evhttp_uri_get_scheme(uri);
    if (!scheme)
    {

        return -1;
    }

    int port = evhttp_uri_get_port(uri);
    if (port < 0)
    {
        if (strcmp(scheme, "http") == 0)
            port = 80;
    }

    const char *host = evhttp_uri_get_host(uri);
    if (!host)
    {
        return -1;
    }

    const char *path = evhttp_uri_get_path(uri);
    if (!path || strlen(path) == 0)
    {
        path = "/";
    }

    const char *query = evhttp_uri_get_query(uri);

    // bufferevent  连接http服务器
    bufferevent *bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);

    // 创建一个 `evhttp_connection` 对象，表示基于网络连接的 HTTP 客户端
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(_base,
                                                                      NULL, bev, host, port);

    // http client  请求 回调函数设置
    evhttp_request *req = evhttp_request_new(http_heartbeat_callback, this);

    // 设置请求的head 消息报头 信息
    evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Host", host);
    evhttp_add_header(output_headers, "Content-Type", " application/json");
    // 发送post数据
    evbuffer *output = evhttp_request_get_output_buffer(req);

    build_heartbeat_msg();

    evbuffer_add_printf(output, "%s", _heart_beat_msg.c_str());

    evhttp_make_request(evcon, req, EVHTTP_REQ_POST, path);

    return 0;
}

void http_heartbeat::http_heartbeat_callback(evhttp_request *req, void *arg)
{

    int code = evhttp_request_get_response_code(req);

    if (code == HTTP_OK)
    {
        return;
    }
}

int http_heartbeat::get_sys_info()
{

    _PID = getpid();

    struct ifaddrs *ifaddr;
    getifaddrs(&ifaddr); // 获取网络接口信息列表

    for (struct ifaddrs *addr = ifaddr; addr != nullptr; addr = addr->ifa_next)
    {
        int family = addr->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            char ip[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &((sockaddr_in *)addr->ifa_addr)->sin_addr, ip, INET_ADDRSTRLEN);
            // std::cout << "Interface: " << addr->ifa_name << ", IP Address: " << ip << std::endl;

            _IP = ip;
        }
    }

    freeifaddrs(ifaddr); // 释放内存资源

    return 0;
}

int http_heartbeat::build_heartbeat_msg()
{

    _onlineTime = time(0) - _start_tm;
    auto time = timeget();
    _gnssecond = time2gpst(time, NULL);

    //{"PID":13996,"gpssecond":64534,"module":"CDC_Plus","node":"192.168.2.167","onlineServer":1234,"onlineTime":1386400,"onlineclient":2345}

    json msg;

    msg["PID"] = _PID;
    msg["gpssecond"] = _gnssecond;
    msg["module"] = _module;
    msg["node"] = _IP;
    msg["onlineServer"] = _onlineServer;
    msg["onlineclient"] = _onlineclient;
    msg["onlineTime"] = _onlineTime;

    _heart_beat_msg = msg.dump();

    return 0;
}

int http_heartbeat::update_info(int Server_count, int Client_count)
{

    _onlineServer = Server_count;
    _onlineclient = Client_count;

    return 0;
}

http_heartbeat::http_heartbeat(/* args */)
{

    _start_tm = time(0);

    get_sys_info();

    // 获取PID

    // 获取IP地址

    //
}

http_heartbeat::~http_heartbeat()
{
}

int http_heartbeat::set_event_base(event_base *base)
{
    _base = base;

    return 0;
}

int http_heartbeat::set_url(const char *url)
{

    _url = url;

    // http://140.207.166.210:9030/DiffBase/moduleInfo/cdcPlus

    return 0;
}