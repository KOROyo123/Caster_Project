#include "client_ntrip.h"
#include "knt/knt.h"
#include <iostream>

#define __class__ "client_ntrip"

client_ntrip::client_ntrip(json req, bufferevent *bev)
{
    _conf = req["Settings"];
    _info = req;
    _bev = bev;

    _user_name = req["user_name"];
    _mount_point = req["mount_point"];
    _connect_key = req["connect_key"];
    if (req["ntrip_version"] == "Ntrip/2.0")
    {
        _NtripVersion2 = true;
        _transfer_with_chunked = true;
    }

    int fd = bufferevent_getfd(_bev);
    _ip = util_get_user_ip(fd);
    _port = util_get_user_port(fd);

    _send_evbuf = evbuffer_new();
    _recv_evbuf = evbuffer_new();

    _connect_timeout = _conf["Timeout"];
    _unsend_limit = _conf["Unsend_Limit"];
}

client_ntrip::~client_ntrip()
{
    // auto fd = bufferevent_getfd(_bev);
    // evutil_closesocket(fd);
    bufferevent_free(_bev);
    evbuffer_free(_send_evbuf);
    evbuffer_free(_recv_evbuf);
}

int client_ntrip::start()
{
    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);

    AUTH::Add_Login_Record(_user_name.c_str(), _connect_key.c_str(), Auth_Login_Callback, this);

    return 0;
}

int client_ntrip::runing()
{
    bufferevent_enable(_bev, EV_READ);

    if (_connect_timeout > 0)
    {
        _bev_read_timeout_tv.tv_sec = _connect_timeout;
        _bev_read_timeout_tv.tv_usec = 0;
        bufferevent_set_timeouts(_bev, &_bev_read_timeout_tv, NULL);
    }

    bev_send_reply();

    // 添加一个请求，订阅指定频道数据
    CASTER::Set_Rover_Client_State_ONLINE(_mount_point.c_str(), NULL, _connect_key.c_str());
    CASTER::Sub_Base_Station_Raw_Data(_mount_point.c_str(), _connect_key.c_str(), Caster_Sub_Callback, this);

    spdlog::info("[{}]: user [{}] is login, using mount [{}], addr:[{}:{}]", __class__, _user_name, _mount_point, _ip, _port);

    return 0;
}

int client_ntrip::stop()
{
    bufferevent_disable(_bev, EV_READ);

    json close_req;
    close_req["origin_req"] = _info;
    close_req["req_type"] = CLOSE_NTRIP_CLIENT;
    QUEUE::Push(close_req);

    CASTER::Set_Rover_Client_State_OFFLINE(_mount_point.c_str(), NULL, _connect_key.c_str());
    CASTER::UnSub_Base_Station_Raw_Data(_mount_point.c_str(), _connect_key.c_str());

    spdlog::info("[{}]: user [{}] is logout, using mount [{}], addr:[{}:{}]", __class__, _user_name, _mount_point, _ip, _port);

    return 0;
}
int client_ntrip::bev_send_reply()
{
    if (_NtripVersion2)
    {
        evbuffer_add_printf(_send_evbuf, "HTTP/1.1 200 OK\r\n");
        evbuffer_add_printf(_send_evbuf, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(_send_evbuf, "Server: Ntrip ExampleCaster/2.0\r\n");
        evbuffer_add_printf(_send_evbuf, "Date: %s\r\n", util_get_http_date().c_str());
        evbuffer_add_printf(_send_evbuf, "Cache-Control: no-store, no-cache, max-age=0\r\n");
        evbuffer_add_printf(_send_evbuf, "Pragma: no-cache\r\n");
        evbuffer_add_printf(_send_evbuf, "Connection: close\r\n");
        evbuffer_add_printf(_send_evbuf, "Content-Type: gnss/data\r\n");
        evbuffer_add_printf(_send_evbuf, "Transfer-Encoding: chunked\r\n");
        evbuffer_add_printf(_send_evbuf, "\r\n");
    }
    else
    {
        evbuffer_add_printf(_send_evbuf, "ICY 200 OK\r\n");
        evbuffer_add_printf(_send_evbuf, "\r\n");
    }

    bufferevent_write_buffer(_bev, _send_evbuf);
    return 0;
}

void client_ntrip::ReadCallback(bufferevent *bev, void *arg)
{
    auto svr = static_cast<client_ntrip *>(arg);
    bufferevent_read_buffer(bev, svr->_recv_evbuf);
    svr->publish_recv_raw_data();
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

int client_ntrip::transfer_sub_raw_data(const char *data, size_t length)
{
    auto UnsendBufferSize = evbuffer_get_length(bufferevent_get_output(_bev));

    if (UnsendBufferSize > _unsend_limit)
    {
        spdlog::info("Client Info: send to user [{}]'s date unsend size is too large :[{}], close the connect! using mount [{}], addr:[{}:{}]", _user_name, UnsendBufferSize, _mount_point, _ip, _port);
        stop();
        return -1;
    }

    if (_transfer_with_chunked)
    {
        evbuffer_add_printf(_send_evbuf, "%lx\r\n", length);
        evbuffer_add(_send_evbuf, "\r\n", 2);
        bufferevent_write_buffer(_bev, _send_evbuf);
    }
    else
    {
        evbuffer_add(_send_evbuf, data, length);
        bufferevent_write_buffer(_bev, _send_evbuf);
    }
    return 0;
}

int client_ntrip::publish_recv_raw_data()
{
    size_t length = evbuffer_get_length(_recv_evbuf);
    char *data = new char[length + 1];
    data[length] = '\0';
    evbuffer_remove(_recv_evbuf, data, length);

    CASTER::Pub_Rover_Client_Raw_Data(_mount_point.c_str(), data, length);

    delete[] data;
    return 0;
}

void client_ntrip::Auth_Login_Callback(const char *request, void *arg, AuthReply *reply)
{
    auto svr = static_cast<client_ntrip *>(arg);

    if (reply->type == AUTH_REPLY_OK)
    {
        svr->runing();
    }
    else
    {
        svr->stop();
    }
}

void client_ntrip::Caster_Sub_Callback(const char *request, void *arg, CatserReply *reply)
{
    auto svr = static_cast<client_ntrip *>(arg);

    if (reply->type == CASTER_REPLY_STRING)
    {
        svr->transfer_sub_raw_data(reply->str, reply->len);
    }
    else if (reply->type == CASTER_REPLY_ERR)
    {
        svr->stop();
    }
}
