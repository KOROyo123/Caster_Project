#include "client_ntrip.h"
#include "knt/knt.h"
#include <iostream>

#define __class__ "client_ntrip"

client_ntrip::client_ntrip(json req, bufferevent *bev, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context)
{
    _info = req;

    _user_name = req["user_name"];
    _mount_point = req["mount_point"];
    _connect_key = req["connect_key"];

    _bev = bev;

    int fd = bufferevent_getfd(_bev);
    _ip = util_get_user_ip(fd);
    _port = util_get_user_port(fd);

    if (req["ntrip_version"] == "Ntrip/2.0")
    {
        _NtripVersion2 = true;
        _transfer_with_chunked = true;
    }

    _pub_context = pub_context;
    _queue = queue;

    _evbuf = evbuffer_new();
}

client_ntrip::~client_ntrip()
{
    auto fd = bufferevent_getfd(_bev);
    bufferevent_free(_bev);
    evutil_closesocket(fd);

    evbuffer_free(_evbuf);
}

int client_ntrip::start()
{

    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
    bufferevent_enable(_bev, EV_READ | EV_WRITE);

    bev_send_reply();

    spdlog::info("Client Info: user [{}] is login, using mount [{}], addr:[{}:{}]", _user_name, _mount_point, _ip, _port);

    return 0;
}

int client_ntrip::stop()
{
    bufferevent_disable(_bev, EV_READ);

    json close_req;
    close_req["origin_req"] = _info;

    _queue->push_and_active(close_req, CLOSE_NTRIP_CLIENT);

    spdlog::info("Client Info: user [{}] is logout, using mount [{}], addr:[{}:{}]", _user_name, _mount_point, _ip, _port);
    return 0;
}


int client_ntrip::bev_send_reply()
{
    if (_NtripVersion2)
    {
        evbuffer_add_printf(_evbuf, "HTTP/1.1 200 OK\r\n");
        evbuffer_add_printf(_evbuf, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(_evbuf, "Server: Ntrip ExampleCaster/2.0\r\n");
        evbuffer_add_printf(_evbuf, "Date: %s\r\n", util_get_http_date().c_str());
        evbuffer_add_printf(_evbuf, "Cache-Control: no-store, no-cache, max-age=0\r\n");
        evbuffer_add_printf(_evbuf, "Pragma: no-cache\r\n");
        evbuffer_add_printf(_evbuf, "Connection: close\r\n");
        evbuffer_add_printf(_evbuf, "Content-Type: gnss/data\r\n");
        evbuffer_add_printf(_evbuf, "Transfer-Encoding: chunked\r\n");
        evbuffer_add_printf(_evbuf, "\r\n");
    }
    else
    {
        evbuffer_add_printf(_evbuf, "ICY 200 OK\r\n");
        evbuffer_add_printf(_evbuf, "\r\n");
    }

    bufferevent_write_buffer(_bev, _evbuf);
    return 0;
}

void client_ntrip::ReadCallback(bufferevent *bev, void *arg)
{
    auto svr = static_cast<client_ntrip *>(arg);
    bufferevent_read_buffer(bev, svr->_evbuf);
    svr->publish_data_from_evbuf();
}

void client_ntrip::EventCallback(bufferevent *bev, short events, void *arg)
{
    auto svr = static_cast<client_ntrip *>(arg);

    spdlog::info("[{}:{}]: {}{}{}{}{}{} , user [{}], mount [{}], addr:[{}:{}]",
                 __class__, __func__,
                 (events & BEV_EVENT_READING) ? "read" : "-",
                 (events & BEV_EVENT_WRITING) ? "write" : "-",
                 (events & BEV_EVENT_EOF) ? "eof" : "-",
                 (events & BEV_EVENT_ERROR) ? "error" : "-",
                 (events & BEV_EVENT_TIMEOUT) ? "timeout" : "-",
                 (events & BEV_EVENT_CONNECTED) ? "connected" : "-", svr->_user_name, svr->_mount_point, svr->_ip, svr->_port);

    svr->stop();
}

void client_ntrip::Evhttp_Close_Callback(evhttp_connection *evcon, void *arg)
{
    auto svr = static_cast<client_ntrip *>(arg);
    svr->stop();
}

int client_ntrip::data_transfer(evbuffer *evbuf)
{

    auto UnsendBufferSize = evbuffer_get_length(bufferevent_get_output(_bev));

    if (UnsendBufferSize > 4096 * 20)
    {
        spdlog::info("Client Info: send to user [{}]'s date unsend size is too large :[{}], close the connect! using mount [{}], addr:[{}:{}]", _user_name, UnsendBufferSize, _mount_point, _ip, _port);
        stop();
        return -1;
    }

    if (_transfer_with_chunked)
    {
        evbuffer_add_printf(_evbuf, "%x\r\n", (unsigned)evbuffer_get_length(evbuf));
        evbuffer_add_buffer_reference(_evbuf, evbuf);
        evbuffer_add(_evbuf, "\r\n", 2);
        bufferevent_write_buffer(_bev, _evbuf);
    }
    else
    {
        bufferevent_write_buffer(_bev, evbuf);
    }
    return 0;
}

int client_ntrip::publish_data_from_evbuf()
{
    size_t length = evbuffer_get_length(_evbuf);
    char *data = new char[length + 1];
    data[length] = '\0';
    evbuffer_remove(_evbuf, data, length);
    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH CSTR_%s %b", _connect_key.c_str(), data, length);
    free(data);
    return 0;
}
