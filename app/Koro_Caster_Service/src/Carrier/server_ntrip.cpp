#include "server_ntrip.h"
#include "knt/knt.h"
#define __class__ "server_ntrip"

server_ntrip::server_ntrip(json req, bufferevent *bev)
{
    _conf = req["Settings"];
    _info = req;
    _bev = bev;

    int fd = bufferevent_getfd(_bev);
    _ip = util_get_user_ip(fd);
    _port = util_get_user_port(fd);

    if (req["ntrip_version"] == "Ntrip/2.0")
    {
        _NtripVersion2 = true;
        _transfer_with_chunked = true;
    }

    _send_evbuf = evbuffer_new();
    _recv_evbuf = evbuffer_new();

    _user_name = req["user_name"];
    _connect_key = _info["connect_key"];
    _mount_point = _info["mount_point"];

    _connect_timeout = _conf["Timeout"];
    _heart_beat_interval = _conf["Heartbeat_Intv"];
    _heart_beat_msg = _conf["Heartbeat_Msg"];
}

server_ntrip::~server_ntrip()
{
    bufferevent_free(_bev);
    evbuffer_free(_send_evbuf);
    evbuffer_free(_recv_evbuf);

    spdlog::info("[{}]: delete mount [{}], addr:[{}:{}]", __class__, _mount_point, _ip, _port);
}

int server_ntrip::start()
{
    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);

    AUTH::Add_Login_Record(_user_name.c_str(), _connect_key.c_str(), Auth_Login_Callback, this);

    return 0;
}

int server_ntrip::stop()
{
    if (_heart_beat_interval > 0)
    {
        event_del(_timeout_ev);
        event_free(_timeout_ev);
    }

    bufferevent_disable(_bev, EV_READ);

    // 向xx发送销毁请求
    json close_req;
    close_req["origin_req"] = _info;
    close_req["req_type"] = CLOSE_NTRIP_SERVER;
    QUEUE::Push(close_req);

    CASTER::Set_Base_Station_State_OFFLINE(_mount_point.c_str(), NULL, _connect_key.c_str());
    AUTH::Add_Logout_Record(_user_name.c_str(), _connect_key.c_str());

    spdlog::info("[{}]: mount [{}] is offline, addr:[{}:{}]", __class__, _mount_point, _ip, _port);

    return 0;
}

int server_ntrip::runing()
{
    bufferevent_enable(_bev, EV_READ | EV_WRITE);

    if (_connect_timeout > 0)
    {
        _bev_read_timeout_tv.tv_sec = _connect_timeout;
        _bev_read_timeout_tv.tv_usec = 0;
        bufferevent_set_timeouts(_bev, &_bev_read_timeout_tv, NULL);
    }

    bev_send_reply();

    CASTER::Set_Base_Station_State_ONLINE(_mount_point.c_str(), _user_name.c_str(), _connect_key.c_str());

    spdlog::info("[{}]: mount [{}] is online, addr:[{}:{}]", __class__, _mount_point, _ip, _port);

    if (_heart_beat_interval > 0)
    {
        _timeout_tv.tv_sec = _heart_beat_interval;
        _timeout_tv.tv_usec = 0;
        _timeout_ev = event_new(bufferevent_get_base(_bev), -1, EV_PERSIST, TimeoutCallback, this);
        event_add(_timeout_ev, &_timeout_tv);

        send_heart_beat_to_server();//直接就触发一次心跳包
    }

    return 0;
}

int server_ntrip::bev_send_reply()
{
    if (_NtripVersion2)
    {
        evbuffer_add_printf(_send_evbuf, "HTTP/1.1 200 OK\r\n");
        evbuffer_add_printf(_send_evbuf, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(_send_evbuf, "Server: Ntrip ExampleCaster/2.0\r\n");
        evbuffer_add_printf(_send_evbuf, "Date: %s\r\n", util_get_http_date().c_str());
        evbuffer_add_printf(_send_evbuf, "Connection: close\r\n");
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

void server_ntrip::ReadCallback(bufferevent *bev, void *arg)
{
    auto svr = static_cast<server_ntrip *>(arg);
    bufferevent_read_buffer(bev, svr->_recv_evbuf);
    svr->publish_recv_raw_data();
}

void server_ntrip::EventCallback(bufferevent *bev, short events, void *arg)
{
    auto svr = static_cast<server_ntrip *>(arg);

    spdlog::info("[{}:{}]: {}{}{}{}{}{} , mount [{}], addr:[{}:{}]",
                 __class__, __func__,
                 (events & BEV_EVENT_READING) ? "read" : "-",
                 (events & BEV_EVENT_WRITING) ? "write" : "-",
                 (events & BEV_EVENT_EOF) ? "eof" : "-",
                 (events & BEV_EVENT_ERROR) ? "error" : "-",
                 (events & BEV_EVENT_TIMEOUT) ? "timeout" : "-",
                 (events & BEV_EVENT_CONNECTED) ? "connected" : "-", svr->_mount_point, svr->_ip, svr->_port);

    svr->stop();
}

void server_ntrip::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto *svr = static_cast<server_ntrip *>(arg);
    svr->send_heart_beat_to_server();
}

int server_ntrip::send_heart_beat_to_server()
{
    bufferevent_write(_bev, _heart_beat_msg.data(), _heart_beat_msg.size());
    return 0;
}

int server_ntrip::publish_recv_raw_data()
{
    if (_transfer_with_chunked)
    {
        publish_data_from_chunck();
    }
    else
    {
        publish_data_from_evbuf();
    }
    return 0;
}

int server_ntrip::publish_data_from_chunck()
{
    if (_chuncked_size == 0)
    {
        // 先读取一行
        char *chunck_head_data;
        size_t chunck_head_size;
        chunck_head_data = evbuffer_readln(_recv_evbuf, &chunck_head_size, EVBUFFER_EOL_CRLF);

        if (!chunck_head_data)
        {
            spdlog::warn("[{}:{}: chunked data error,close connect! {},{},{}", __class__, __func__, _mount_point, _ip, _port);
            stop();
            return 1;
        }
        sscanf(chunck_head_data, "%lx", &chunck_head_size);

        _chuncked_size = chunck_head_size;

        // 读取一行
        // 获取chunck长度 更新chuncked_size
    }

    // 判断长剩余长度是否满足chunck长度（即块数据都已接收到）

    size_t length = evbuffer_get_length(_recv_evbuf);

    if (_chuncked_size + 2 <= length) // 还有回车换行
    {
        char *data = new char[_chuncked_size + 3];
        data[_chuncked_size + 2] = '\0';

        evbuffer_remove(_recv_evbuf, data, _chuncked_size);
        CASTER::Pub_Base_Station_Raw_Data(_mount_point.c_str(), data, _chuncked_size);

        _chuncked_size = 0;
        delete[] data;
    }
    else
    {
        return 1;
        // 不满足，记录当前chunck长度，等待后续数据来了再发送
    }

    // 如果evbuffer中还有未发送的数据，那就再进行一次函数
    if (evbuffer_get_length(_recv_evbuf) > 0)
    {
        publish_data_from_chunck();
    }

    return 0;
}

int server_ntrip::publish_data_from_evbuf()
{
    size_t length = evbuffer_get_length(_recv_evbuf);

    char *data = new char[length + 1];
    data[length] = '\0';

    evbuffer_remove(_recv_evbuf, data, length);
    CASTER::Pub_Base_Station_Raw_Data(_mount_point.c_str(), data, length);

    delete[] data;
    return 0;
}

void server_ntrip::Auth_Login_Callback(const char *request, void *arg, AuthReply *reply)
{
    auto svr = static_cast<server_ntrip *>(arg);
    if (reply->type == AUTH_REPLY_OK)
    {
        svr->runing();
    }
    else
    {
        spdlog::info("[{}]: AUTH_REPLY_ERROR user [{}] , using mount [{}], addr:[{}:{}]", __class__, svr->_user_name, svr->_mount_point, svr->_ip, svr->_port);
        svr->stop();
    }
}
