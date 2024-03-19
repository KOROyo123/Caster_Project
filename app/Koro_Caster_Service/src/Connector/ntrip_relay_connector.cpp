#include "ntrip_relay_connector.h"

#define __class__ "ntrip_relay_connector"

ntrip_relay_connector::ntrip_relay_connector(event_base *base, std::shared_ptr<process_queue> queue, std::unordered_map<std::string, bufferevent *> *connect_map, redisAsyncContext *pub_context)
{
    _base = base;
    _queue = queue;
    _connect_map = connect_map;
    _pub_context = pub_context;
}

ntrip_relay_connector::~ntrip_relay_connector()
{
    _req_map.clear();

}

int ntrip_relay_connector::start()
{
    return 0;
}

int ntrip_relay_connector::stop()
{
    return 0;
}

std::string ntrip_relay_connector::create_new_connection(json con_info)
{
    std::string source_addr = con_info["addr"];
    int port = con_info["port"];
    std::string source_port = std::to_string(port);

    bufferevent *bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd

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
        // 连接建立失败
        bufferevent_free(bev);
        return std::string();
    }

    // 连接建立成功。返回port，连接建立失败，返回0
    auto fd = bufferevent_getfd(bev);
    struct sockaddr_in sa;
    socklen_t len = sizeof(sa);
    if (getsockname(fd, (struct sockaddr *)&sa, &len))
    {
        // 获取本地连接信息失败
        bufferevent_free(bev);
        return std::string();
    }

    std::string Connect_Key = util_cal_connect_key(fd);
    if(Connect_Key.empty())
    {
        bufferevent_free(bev);
        return std::string();
    }

    con_info["req_type"] = CREATE_RELAY_SERVER;
    con_info["connect_key"] = Connect_Key;

    std::string intel_mount = con_info["Maping_Mount"];
    intel_mount += "-" + util_port_to_key(ntohs(sa.sin_port));
    con_info["mount_point"] = intel_mount;
    con_info["mount_group"] = "usr_relay";

    auto arg = new std::pair<ntrip_relay_connector *, std::string>(this, Connect_Key);
    bufferevent_setcb(bev, ReadCallback, NULL, EventCallback, arg);
    bufferevent_enable(bev, EV_READ);

    _connect_map->insert(std::make_pair(Connect_Key, bev));
    _req_map.insert(std::make_pair(Connect_Key, con_info));

    return intel_mount;
}

void ntrip_relay_connector::EventCallback(bufferevent *bev, short events, void *arg)
{
    auto ctx = static_cast<std::pair<ntrip_relay_connector *, std::string> *>(arg);

    ntrip_relay_connector *svr = ctx->first;
    std::string key = ctx->second;
    // 如果是连接建立成功，发送验证消息
    // 连接建立成功
    if (events == BEV_EVENT_CONNECTED)
    {
        svr->send_login_request(bev, key);
        return;
    }

    spdlog::info("[{}:{}]: {}{}{}{}{}{}",
                 __class__, __func__,
                 (events & BEV_EVENT_READING) ? "read" : "-",
                 (events & BEV_EVENT_WRITING) ? "write" : "-",
                 (events & BEV_EVENT_EOF) ? "eof" : "-",
                 (events & BEV_EVENT_ERROR) ? "error" : "-",
                 (events & BEV_EVENT_TIMEOUT) ? "timeout" : "-",
                 (events & BEV_EVENT_CONNECTED) ? "connected" : "-");

    bufferevent_free(bev);

    svr->request_give_back_account(key);

    //  如果是连接建立失败，关闭连接，移除
}

void ntrip_relay_connector::ReadCallback(bufferevent *bev, void *arg)
{
    auto ctx = static_cast<std::pair<ntrip_relay_connector *, std::string> *>(arg);
    ntrip_relay_connector *svr = ctx->first;
    std::string key = ctx->second;

    if (svr->verify_login_response(bev, key))
    {
        spdlog::warn("[{}:{}]: verify login response fail", __class__, __func__);
        return;
    }

    svr->request_new_relay_server(key);
}

int ntrip_relay_connector::send_login_request(bufferevent *bev, std::string Conncet_Key)
{
    evbuffer *evbuf = bufferevent_get_output(bev);

    auto item = _req_map.find(Conncet_Key);
    json con_info = item->second;
    std::string mount = con_info["Mount"];
    std::string NtripVersion = con_info["Ntrip-Version"];
    std::string addr = con_info["addr"];
    int port = con_info["port"];
    std::string usr_pwd = con_info["usr_pwd"];
    std::string userID = util_base64_encode(usr_pwd.c_str());

    if (NtripVersion == "Ntrip/2.0")
    {
        evbuffer_add_printf(evbuf, "GET %s HTTP/1.1\r\n", mount.c_str());
        evbuffer_add_printf(evbuf, "Host: %s:%d\r\n", addr.c_str(), port);
        evbuffer_add_printf(evbuf, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(evbuf, "User-Agent: %s/%s\r\n", PROJECT_SET_NAME, PROJECT_SET_VERSION);
        evbuffer_add_printf(evbuf, "Authorization: Basic %s\r\n", userID.c_str());
        evbuffer_add_printf(evbuf, "Connection: close\r\n");
        evbuffer_add_printf(evbuf, "\r\n");
    }
    else
    {
        evbuffer_add_printf(evbuf, "GET %s HTTP/1.0\r\n", mount.c_str());
        evbuffer_add_printf(evbuf, "User-Agent: %s/%s\r\n", PROJECT_SET_NAME, PROJECT_SET_VERSION);
        evbuffer_add_printf(evbuf, "Authorization: Basic %s\r\n", userID.c_str());
        evbuffer_add_printf(evbuf, "\r\n");
    }

    return 0;
}

int ntrip_relay_connector::verify_login_response(bufferevent *bev, std::string Conncet_Key)
{
    evbuffer *evbuf = bufferevent_get_input(bev);

    auto item = _req_map.find(Conncet_Key);
    json con_info = item->second;
    // std::string mount = con_info["Mount"];
    std::string NtripVersion = con_info["Ntrip-Version"];
    // std::string addr = con_info["addr"];
    // int port = con_info["port"];
    // std::string usr_pwd = con_info["username_pwd"];
    // std::string userID = util_base64_encode(usr_pwd.c_str());

    size_t headerlen = 0;
    char *header = evbuffer_readln(evbuf, &headerlen, EVBUFFER_EOL_CRLF_STRICT);

    if (header == NULL | headerlen > 128)
    {
        spdlog::warn("[{}:{}]: error respone", __class__, __func__);
        return 1;
    }

    if (NtripVersion == "Ntrip/2.0")
    {
        if (strcmp(header, "HTTP/1.1 200 OK"))
        {
            spdlog::warn("[{}:{}]: Unexpected respone", __class__, __func__);
            return 1;
        }
    }
    else
    {
        if (strcmp(header, "ICY 200 OK"))
        {
            spdlog::warn("[{}:{}]: Unexpected respone", __class__, __func__);
            return 1;
        }
    }

    free(header);
    evbuffer_drain(evbuf, evbuffer_get_length(evbuf));

    return 0;
}

int ntrip_relay_connector::request_new_relay_server(std::string Conncet_Key)
{
    auto item = _req_map.find(Conncet_Key);
    auto req = item->second;

    _queue->push_and_active(req, req["req_type"]);

    return 0;
}

int ntrip_relay_connector::request_give_back_account(std::string Conncet_Key)
{
    _connect_map->erase(Conncet_Key);

    auto req = _req_map.find(Conncet_Key);

    json back_account_req;
    back_account_req["origin_req"] = req->second;
    _queue->push_and_active(back_account_req, CLOSE_RELAY_REQ_CONNECT);

    return 0;
}

int ntrip_relay_connector::redis_Info_Record(json req)
{





    
    return 0;
}
