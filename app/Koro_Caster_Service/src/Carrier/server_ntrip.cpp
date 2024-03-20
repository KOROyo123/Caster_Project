#include "server_ntrip.h"
#include "knt/knt.h"
#define __class__ "server_ntrip"

server_ntrip::server_ntrip(json conf, json req, bufferevent *bev, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context)
{
    _conf = conf;
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

    _evbuf = evbuffer_new();

    _pub_context = pub_context;
    _queue = queue;

    _Connect_Key = _info["connect_key"];
    _publish_mount = _info["mount_point"];
    _mount_group = _info["mount_group"];

    _connect_timeout = _conf["Connect_Timeout"];
    _heart_beat_switch = _conf["Heart_Beat_Switch"];
    _heart_beat_interval = _conf["Heart_Beat_Interval"];
    _heart_beat_msg = _conf["Heart_Beat_Msg"];
}

server_ntrip::~server_ntrip()
{
    bufferevent_free(_bev);
    evbuffer_free(_evbuf);
}

std::string server_ntrip::get_connect_key()
{
    return _Connect_Key;
}

int server_ntrip::start()
{

    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
    bufferevent_enable(_bev, EV_READ | EV_WRITE);

    if (_connect_timeout > 0)
    {
        _bev_read_timeout_tv.tv_sec = _connect_timeout;
        _bev_read_timeout_tv.tv_usec = 0;

        bufferevent_set_timeouts(_bev, &_bev_read_timeout_tv, NULL);
    }

    bev_send_reply();

    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH mp_online %s ", _publish_mount.c_str());
    redisAsyncCommand(_pub_context, NULL, NULL, "HSET mp_ol_all %s %s", _publish_mount.c_str(), _Connect_Key.c_str());
    redisAsyncCommand(_pub_context, NULL, NULL, "HSET mp_ol_%s %s %s", _mount_group.c_str(), _publish_mount.c_str(), _Connect_Key.c_str());
    // redisAsyncCommand(_pub_context, NULL, NULL, "EXPIRE MOUNT_STATE_%s 30 GT", _publish_mount.c_str());

    spdlog::info("Mount Info: mount [{}] is online, addr:[{}:{}]", _publish_mount, _ip, _port);

    _timeout_tv.tv_sec = _heart_beat_interval;
    _timeout_tv.tv_usec = 0;
    _timeout_ev = event_new(bufferevent_get_base(_bev), -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_timeout_ev, &_timeout_tv);

    return 0;
}

int server_ntrip::stop()
{
    event_del(_timeout_ev);
    event_free(_timeout_ev);

    bufferevent_disable(_bev, EV_READ);

    // 向xx发送销毁请求
    json close_req;
    close_req["origin_req"] = _info;
    _queue->push_and_active(close_req, CLOSE_NTRIP_SERVER);

    spdlog::info("Mount Info: mount [{}] is offline, addr:[{}:{}]", _publish_mount, _ip, _port);

    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH mp_offline %s ", _publish_mount.c_str());
    redisAsyncCommand(_pub_context, NULL, NULL, "HDEL mp_ol_all %s ", _publish_mount.c_str());
    redisAsyncCommand(_pub_context, NULL, NULL, "HDEL mp_ol_%s %s ", _mount_group.c_str(), _publish_mount.c_str());

    return 0;
}

int server_ntrip::bev_send_reply()
{

    if (_NtripVersion2)
    {
        evbuffer_add_printf(_evbuf, "HTTP/1.1 200 OK\r\n");
        evbuffer_add_printf(_evbuf, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(_evbuf, "Server: Ntrip ExampleCaster/2.0\r\n");
        evbuffer_add_printf(_evbuf, "Date: %s\r\n", util_get_http_date().c_str());
        evbuffer_add_printf(_evbuf, "Connection: close\r\n");
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

void server_ntrip::ReadCallback(bufferevent *bev, void *arg)
{
    auto svr = static_cast<server_ntrip *>(arg);

    bufferevent_read_buffer(bev, svr->_evbuf);

    if (svr->_transfer_with_chunked)
    {
        svr->publish_data_from_chunck();
    }
    else
    {
        svr->publish_data_from_evbuf();
    }
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
                 (events & BEV_EVENT_CONNECTED) ? "connected" : "-", svr->_publish_mount, svr->_ip, svr->_port);

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

int server_ntrip::publish_data_from_chunck()
{
    if (_chuncked_size == 0)
    {
        // 先读取一行
        char *chunck_head_data;
        size_t chunck_head_size;
        chunck_head_data = evbuffer_readln(_evbuf, &chunck_head_size, EVBUFFER_EOL_CRLF);

        if (!chunck_head_data)
        {
            spdlog::warn("[{}:{}: chunked data error,close connect! {},{},{}", __class__, __func__, _publish_mount, _ip, _port);
            stop();
        }
        sscanf(chunck_head_data, "%lx", &chunck_head_size);

        _chuncked_size = chunck_head_size;

        // 读取一行
        // 获取chunck长度 更新chuncked_size
    }

    // 判断长剩余长度是否满足chunck长度（即块数据都已接收到）

    size_t length = evbuffer_get_length(_evbuf);

    if (_chuncked_size + 2 <= length) // 还有回车换行
    {
        unsigned char *data = new unsigned char[_chuncked_size + 3];
        data[_chuncked_size + 2] = '\0';

        evbuffer_remove(_evbuf, data, _chuncked_size);
        redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH STR_%s %b", _publish_mount.c_str(), data, _chuncked_size);

        _chuncked_size = 0;
        delete[] data;
    }
    else
    {
        return 1;
        // 不满足，记录当前chunck长度，等待后续数据来了再发送
    }

    // 如果evbuffer中还有未发送的数据，那就再进行一次函数
    if (evbuffer_get_length(_evbuf) > 0)
    {
        publish_data_from_chunck();
    }

    return 0;
}

int server_ntrip::publish_data_from_evbuf()
{
    size_t length = evbuffer_get_length(_evbuf);

    unsigned char *data = new unsigned char[length + 1];
    data[length] = '\0';

    evbuffer_remove(_evbuf, data, length);

    redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH STR_%s %b", _publish_mount.c_str(), data, length);

    delete[] data;

    return 0;
}