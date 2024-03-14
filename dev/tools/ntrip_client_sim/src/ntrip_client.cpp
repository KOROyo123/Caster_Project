#include "ntrip_client.h"
#include "event2/util.h"
#include <cstring>

#include "knt/base64.h"




ntrip_client::ntrip_client(event_base *base)
{
    _base = base;
    _bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
}

ntrip_client::~ntrip_client()
{
    bufferevent_free(_bev);
}

int ntrip_client::connect(std::string mountpoint, std::string userID, std::string host, std::string server, bool ntrip2)
{
    _mountpoint = mountpoint;
    _userID = userID;
    _host = host;
    _server = server;
    _ntrip2 = ntrip2;

    evutil_addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    evutil_getaddrinfo(host.c_str(), server.c_str(), &hints, &res);

    // 创建一个绑定在base上的buffevent，并建立socket连接

    if (bufferevent_socket_connect(_bev, res->ai_addr, res->ai_addrlen))
    {
        return 1;
    }

    bufferevent_setcb(_bev, VeifyCallback, NULL, ConnectCallback, this);
    bufferevent_enable(_bev, EV_READ);

    return 0;
}

void ntrip_client::ReadCallback(struct bufferevent *bev, void *arg)
{
    ntrip_client *client = static_cast<ntrip_client *>(arg);

    evbuffer *buf = evbuffer_new();

    bufferevent_read_buffer(client->_bev, buf);

    evbuffer_free(buf);
}

void ntrip_client::EventCallback(struct bufferevent *bev, short events, void *arg)
{
    ntrip_client *client = static_cast<ntrip_client *>(arg);

    client->~ntrip_client();
}

void ntrip_client::VeifyCallback(struct bufferevent *bev, void *arg)
{
    ntrip_client *client = static_cast<ntrip_client *>(arg);

    evbuffer *buf = evbuffer_new();

    bufferevent_read_buffer(client->_bev, buf);

    char *recv = evbuffer_readline(buf);

    if (client->_ntrip2)
    {
        if (strcmp(recv, "HTTP/1.1 200 OK"))
        {
            client->~ntrip_client();
            return;
        }
    }
    else
    {
        if (strcmp(recv, "ICY 200 OK"))
        {
            client->~ntrip_client();
            return;
        }
    }

    free(recv);
    evbuffer_free(buf);

    bufferevent_setcb(client->_bev, ReadCallback, NULL, EventCallback, client);
}

void ntrip_client::ConnectCallback(struct bufferevent *bev, short events, void *arg)
{
    ntrip_client *client = static_cast<ntrip_client *>(arg);

    if (events == BEV_EVENT_CONNECTED) // 连接建立成功
    {
        evbuffer *output = bufferevent_get_output(client->_bev);

        auto userID=util_base64_encode(client->_userID.c_str());

        if (client->_ntrip2)
        {
            evbuffer_add_printf(output, "GET /%s HTTP/1.1\r\n", client->_mountpoint.c_str());
            evbuffer_add_printf(output, "Host: %s\r\n", client->_host.c_str());
            evbuffer_add_printf(output, "Ntrip-Version: Ntrip/2.0\r\n");
            evbuffer_add_printf(output, "User-Agent: Ntrip Koro_NavTool/2.0\r\n");
            evbuffer_add_printf(output, "Authorization: Basic %s\r\n", userID.c_str());
            evbuffer_add_printf(output, "Connection: close\r\n\r\n");
        }
        else
        {
            evbuffer_add_printf(output, "GET /%s HTTP/1.0\r\n", client->_mountpoint.c_str());
            evbuffer_add_printf(output, "User-Agent: NTRIP Koro_NavTool\r\n");
            evbuffer_add_printf(output, "Authorization: Basic %s\r\n\r\n", userID.c_str());
        }
    }
    else
    {
        client->~ntrip_client();
    }
}

void ntrip_client::TimeoutCallback(evutil_socket_t *bev, short events, void *arg)
{
    ntrip_client *client = static_cast<ntrip_client *>(arg);
}




