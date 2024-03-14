#include "ntrip_server.h"
#include "event2/util.h"
#include <cstring>

#include "knt/base64.h"

ntrip_server::ntrip_server(event_base *base, evbuffer *data_buf)
{
    _base = base;
    _data_buf = data_buf;
    _bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
}

ntrip_server::~ntrip_server()
{

    bufferevent_free(_bev);
    _bev = nullptr;
}

int ntrip_server::connect(std::string mountpoint, std::string user, std::string pwd, std::string host, std::string server, bool ntrip2)
{
    _mountpoint = mountpoint;
    _user = user;
    _pwd = pwd;
    _userID += _user;
    _userID += ":";
    _userID += _pwd;

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

void ntrip_server::ReadCallback(struct bufferevent *bev, void *arg)
{
    ntrip_server *server = static_cast<ntrip_server *>(arg);

    evbuffer *buf = evbuffer_new();

    bufferevent_read_buffer(server->_bev, buf);

    evbuffer_free(buf);
}

void ntrip_server::EventCallback(struct bufferevent *bev, short events, void *arg)
{
    ntrip_server *server = static_cast<ntrip_server *>(arg);

    server->~ntrip_server();
}

void ntrip_server::VeifyCallback(struct bufferevent *bev, void *arg)
{
    ntrip_server *server = static_cast<ntrip_server *>(arg);

    evbuffer *buf = evbuffer_new();

    bufferevent_read_buffer(server->_bev, buf);

    char *recv = evbuffer_readline(buf);

    if (server->_ntrip2)
    {
        if (strcmp(recv, "HTTP/1.1 200 OK"))
        {
            server->~ntrip_server();
            return;
        }
    }
    else
    {
        if (strcmp(recv, "ICY 200 OK") && strcmp(recv, "OK"))
        {
            server->~ntrip_server();
            return;
        }
    }

    free(recv);
    evbuffer_free(buf);

    bufferevent_setcb(server->_bev, ReadCallback, NULL, EventCallback, server);

    server->_sendev = event_new(server->_base, -1, EV_PERSIST, TimeoutCallback, server);

    event_add(server->_sendev, &server->_ev);
}

void ntrip_server::ConnectCallback(struct bufferevent *bev, short events, void *arg)
{
    ntrip_server *server = static_cast<ntrip_server *>(arg);

    if (events == BEV_EVENT_CONNECTED) // 连接建立成功
    {
        evbuffer *output = bufferevent_get_output(server->_bev);
        if (server->_ntrip2)
        {
            evbuffer_add_printf(output, "POST /%s HTTP/1.1\r\n", server->_mountpoint.c_str());
            evbuffer_add_printf(output, "Host: %s\r\n", server->_host.c_str());
            evbuffer_add_printf(output, "Ntrip-Version: Ntrip/2.0\r\n");
            evbuffer_add_printf(output, "Authorization: Basic %s\r\n", util_base64_encode(server->_userID.c_str()).c_str());
            evbuffer_add_printf(output, "User-Agent: Ntrip Koro_NavTool/2.0\r\n");
            evbuffer_add_printf(output, "Connection: close\r\n\r\n");
        }
        else
        {
            evbuffer_add_printf(output, "SOURCE %s /%s\r\n", server->_pwd.c_str(), server->_mountpoint.c_str());
            evbuffer_add_printf(output, "Source-Agent: Ntrip Koro_NavTool/2.0\r\n");
            evbuffer_add_printf(output, "\r\n");
        }
    }
    else
    {
        server->~ntrip_server();
    }
}

void ntrip_server::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    ntrip_server *server = static_cast<ntrip_server *>(arg);

    evbuffer *trans_evbuf = evbuffer_new();

    evbuffer_add_buffer_reference(trans_evbuf, server->_data_buf);

    if (server->_bev)
    {
        bufferevent_write_buffer(server->_bev, trans_evbuf);
    }

    evbuffer_free(trans_evbuf);
}
