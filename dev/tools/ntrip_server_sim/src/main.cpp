#include <iostream>
#include <fstream>
#include "ntrip_server.h"
#include "event2/event.h"
#include <nlohmann/json.hpp>
#include "knt/knt.h"

using json = nlohmann::json;

void RecvDataCallback(struct bufferevent *bev, void *arg)
{
    evbuffer *evbuf = static_cast<evbuffer *>(arg);

    evbuffer *oldbuf = evbuffer_new();

    int size = evbuffer_get_length(evbuf);

    evbuffer_drain(evbuf, 40960);

    bufferevent_read_buffer(bev, evbuf);

    std::cout << "update send data:" << evbuffer_get_length(evbuf) << std::endl;
}

int main()
{

    std::ifstream f("ntrip_server_sim_conf.json");

    if (!f.is_open())
    {
        return 1;
    }

    json list = json::parse(f);

    // 数据源连接

    std::string addr = list["Caster_IP"];
    std::string port = list["Caster_Port"];

    std::string mount = list["Push_Mount_Head"];
    bool autoNO = list["Auto_Add_NO"] == 0 ? false : true;
    int con_num = list["Push_Mount_Num"];
    bool ntrip2 = list["Ntrip_Version"] == 2 ? true : false;

    std::string source_addr = list["Source_IP"];
    std::string source_port = list["Source_Port"];

    event_base *base = event_base_new();
    evbuffer *evbuf = evbuffer_new();

    evbuffer_add_printf(evbuf, "wait for data source init\r\n");

    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd

    evutil_addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    evutil_getaddrinfo(source_addr.c_str(), source_port.c_str(), &hints, &res);

    // 创建一个绑定在base上的buffevent，并建立socket连接

    if (bufferevent_socket_connect(bev, res->ai_addr, res->ai_addrlen))
    {
        return 1;
    }

    bufferevent_setcb(bev, RecvDataCallback, NULL, NULL, evbuf);
    bufferevent_enable(bev, EV_READ);

    for (int i = 0; i < con_num; i++)
    {
        ntrip_server *server = new ntrip_server(base, evbuf);
        if (autoNO)
        {
            server->connect(mount + std::to_string(i), util_random_string(8), util_random_string(8), addr, port, ntrip2);
        }
        else
        {
            server->connect(mount + '-' + util_random_string(8), util_random_string(8), util_random_string(8), addr, port, ntrip2);
        }
    }

    event_base_dispatch(base);

    return 0;
}