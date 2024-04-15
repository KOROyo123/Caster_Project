#include "ntrip_compat_listener.h"

#ifdef WIN32

#else
#include <arpa/inet.h>
#endif

#define __class__ "ntrip_compat_listener"

ntrip_compat_listener::ntrip_compat_listener(event_base *base, std::unordered_map<std::string, bufferevent *> *connect_map)
{
    _base = base;
    _connect_map = connect_map;
}

ntrip_compat_listener::~ntrip_compat_listener()
{
}

int ntrip_compat_listener::set_listen_conf(json conf)
{
    _server_IP = conf["Server_IP"];
    _listen_port = conf["Listener_Port"];

    _connect_timeout = conf["Connect_Timeout"];

    // _Server_Login_With_Password = conf["Server_Login_With_Password"];
    // _Client_Login_With_Password = conf["Client_Login_With_Password"];
    _Nearest_Support = conf["Nearest_Support"];
    _Virtal_Support = conf["Virtal_Support"];

    return 0;
}

int ntrip_compat_listener::start()
{
    struct sockaddr_in sin = {0};

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr("0.0.0.0");
    sin.sin_port = htons(_listen_port);

    _listener = evconnlistener_new_bind(_base, AcceptCallback, this, LEV_OPT_LEAVE_SOCKETS_BLOCKING | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
    evconnlistener_set_error_cb(_listener, AcceptErrorCallback);

    if (!_listener)
    {
        spdlog::error("ntrip listener: couldn't bind to port {}.", _listen_port);
        return 1;
    }

    spdlog::info("ntrip listener: bind to port {} success, start listen...", _listen_port);

    return 0;
}

int ntrip_compat_listener::stop()
{
    evconnlistener_free(_listener);

    spdlog::info("ntrip listener: stop bind port %d , stop listener.", _listen_port);
    return 0;
}

int ntrip_compat_listener::enable_Nearest_Support()
{
    _Nearest_Support = true;
    return 0;
}

int ntrip_compat_listener::disable_Nearest_Support()
{
    _Nearest_Support = false;
    return 0;
}

int ntrip_compat_listener::enable_Virtual_Support()
{
    _Virtal_Support = true;
    return 0;
}

int ntrip_compat_listener::disable_Virtual_Support()
{
    _Virtal_Support = false;
    return 0;
}

int ntrip_compat_listener::add_Virtal_Mount(std::string mount_point)
{
    if (_Virtal_Support)
    {
        _support_virtual_mount.insert(mount_point);
        return 0;
    }
    return 1;
}

int ntrip_compat_listener::del_Virtal_Mount(std::string mount_point)
{
    _support_virtual_mount.erase(mount_point);
    return 0;
}

void ntrip_compat_listener::AcceptCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg)
{
    auto svr = static_cast<ntrip_compat_listener *>(arg);
    event_base *base = svr->_base;

    bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    std::string ip = util_get_user_ip(fd);
    int port = util_get_user_port(fd);
    spdlog::info("[{}]: receive new connect, addr:[{}:{}]", __class__, ip, port);

    std::string Connect_Key = util_cal_connect_key(fd);
    svr->_connect_map->insert(std::make_pair(Connect_Key, bev));

    auto timer = new timeval;
    timer->tv_sec = 30;
    timer->tv_usec = 0;
    svr->_timer_map.insert(std::make_pair(Connect_Key, timer));

    auto ctx = new std::pair<ntrip_compat_listener *, std::string>(svr, Connect_Key);
    bufferevent_setcb(bev, Ntrip_Decode_Request_cb, NULL, Bev_EventCallback, ctx);
    bufferevent_set_timeouts(bev, timer, NULL);
    bufferevent_enable(bev, EV_READ);
}

void ntrip_compat_listener::AcceptErrorCallback(evconnlistener *listener, void *arg)
{
    spdlog::warn("[{}]: listener error!", __class__);

    auto svr = static_cast<ntrip_compat_listener *>(arg);

    // struct event_base *base;
    // base = evconnlistener_get_base(listener);

    //----------------等待验证功能，连接出错后重连-------------------------
    spdlog::info("[{}]:create new listener!", __class__);
    if (listener)
    {
        evconnlistener_free(listener); // 释放旧连接
    }

    if (svr->start()) // 启动新连接
    {
        // 重连失败，那就只好先退出了
        event_base_loopexit(svr->_base, NULL); // TODO:有必要调用此函数么 进程退出么 最后一个参数的意义
    }
}

void ntrip_compat_listener::Ntrip_Decode_Request_cb(bufferevent *bev, void *ctx)
{
    auto arg = static_cast<std::pair<ntrip_compat_listener *, std::string> *>(ctx);
    auto svr = arg->first;
    auto key = arg->second;

    // 已经接收到请求，解析请求即可，这个请求决定了连接是进入下一步还是关闭
    bufferevent_disable(bev, EV_READ);              // 暂停/停止接收数据
    bufferevent_setcb(bev, NULL, NULL, NULL, NULL); // 清空bev绑定的回调？  如果这个时候bev event_cb已经激活怎么办?是否就不继续执行了
    // 已经接收到请求，也需要关闭定时器
    bufferevent_set_timeouts(bev, NULL, NULL); // 解绑定时器
    auto timer = svr->_timer_map.find(key);
    if (timer != svr->_timer_map.end())
    {
        delete timer->second; // 删除定时器
        svr->_timer_map.erase(key);
    }

    int fd = bufferevent_getfd(bev);
    std::string ip = util_get_user_ip(fd);
    int port = util_get_user_port(fd);

    evbuffer *evbuf = bufferevent_get_input(bev);

    size_t header_len = 0;
    char *header = evbuffer_readln(evbuf, &header_len, EVBUFFER_EOL_CRLF);

    try
    {
        if (header == NULL | header_len > 255)
        {
            spdlog::warn("[{}:{}]: error respone, from: [ip: {} port: {}]", __class__, __func__, ip, port);
            throw 1;
        }
        spdlog::info("[{}]: receive request header: [{}], from: [ip: {} port: {}]", __class__, header, ip, port);

        char ele[4][256] = {'\0'};
        sscanf(header, "%[^ |\n] %[^ |\n] %[^ |\n] %[^ |\n]", ele[0], ele[1], ele[2], ele[3]);

        // 判断是否是Server还是Client
        if (strcmp(ele[0], "GET") == 0)
        {
            svr->Process_GET_Request(bev, ele[1]);
        }
        else if (strcmp(ele[0], "POST") == 0)
        {
            svr->Process_POST_Request(bev, ele[1]);
        }
        else if (strcmp(ele[0], "SOURCE") == 0)
        {
            if (strcmp(ele[2], "HTTP/1.1") == 0 | strcmp(ele[2], "HTTP/1.0") == 0) // 针对报文： SOURCE  KOROYO2 HTTP/1.1
            {
                svr->Process_SOURCE_Request(bev, ele[1], "");
            }
            else if (ele[2][0] == '\0') // 针对报文： SOURCE  KOROYO2
            {
                svr->Process_SOURCE_Request(bev, ele[1], "");
            }
            else // 针对报文： SOURCE 42411 KOROYO2 HTTP/1.1 |  SOURCE 42411 KOROYO2
            {
                svr->Process_SOURCE_Request(bev, ele[2], ele[1]);
            }
        }
        else
        {
            // 不支持的方法
            spdlog::info("[{}:{}]: receive unsuppose request", __class__, __func__);
            throw 1;
        }
    }
    catch (int i)
    {
        svr->Process_Unknow_Request(bev);
    }

    // 清理
    delete arg;   // 删除arg
    free(header); // 删除读取的文件头
}

void ntrip_compat_listener::Bev_EventCallback(bufferevent *bev, short events, void *ctx)
{
    auto arg = static_cast<std::pair<ntrip_compat_listener *, std::string> *>(ctx);
    auto svr = arg->first;
    auto key = arg->second;

    if (events == BEV_EVENT_CONNECTED)
    {
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

    // 删除连接bev
    bufferevent_free(bev);
    svr->_connect_map->erase(key);

    // 删除定时器
    auto timer = svr->_timer_map.find(key);
    if (timer != svr->_timer_map.end())
    {
        delete timer->second;
        svr->_timer_map.erase(key);
    }

    delete arg; // 发生事件之后，参数已经没有用，但是是new出来的pair，需要释放
}

int ntrip_compat_listener::Process_GET_Request(bufferevent *bev, const char *url)
{
    json req = decode_bufferevent_req(bev);
    req["mount_point"] = extract_path(url); // 提取请求的?前的内容
    req["mount_para"] = extract_para(url);  // 提取请求的?后的内容

    std::string mount = req["mount_point"];
    if (mount.empty()) // 相当于"/"
    {
        req["req_type"] = REQUEST_SOURCE_LOGIN;
    }
    else if (mount == "NEAREST")
    {
        req["req_type"] = REQUEST_NEAREST_LOGIN;
    }
    else if (_support_virtual_mount.find(mount) != _support_virtual_mount.end())
    {
        req["req_type"] = REQUEST_VIRTUAL_LOGIN;
    }
    else
    {
        req["req_type"] = REQUEST_CLIENT_LOGIN;
    }

    std::string userID = req["user_baseID"];
    auto ctx = new std::pair<ntrip_compat_listener *, json>(this, req);
    AUTH::Verify(userID.c_str(), Auth_Verify_Cb, ctx);
    return 0;
}

int ntrip_compat_listener::Process_POST_Request(bufferevent *bev, const char *url)
{
    json req = decode_bufferevent_req(bev);
    req["mount_point"] = extract_path(url);
    req["mount_para"] = extract_para(url);
    req["req_type"] = REQUEST_SERVER_LOGIN;

    std::string userID = req["user_baseID"];
    auto ctx = new std::pair<ntrip_compat_listener *, json>(this, req);
    AUTH::Verify(userID.c_str(), Auth_Verify_Cb, ctx);
    return 0;
}

int ntrip_compat_listener::Process_SOURCE_Request(bufferevent *bev, const char *url, const char *secret)
{
    json req = decode_bufferevent_req(bev);
    req["mount_point"] = extract_path(url);
    req["mount_para"] = extract_para(url);
    req["req_type"] = REQUEST_SERVER_LOGIN;

    std::string pwd = secret;
    if (pwd != "")
    {
        req["user"] = pwd;
        req["pwd"] = pwd;
    }
    std::string userID = req["user_baseID"];
    auto ctx = new std::pair<ntrip_compat_listener *, json>(this, req);
    AUTH::Verify(userID.c_str(), Auth_Verify_Cb, ctx);
    return 0;
}

int ntrip_compat_listener::Process_Unknow_Request(bufferevent *bev)
{
    erase_and_free_bev(bev);
    return 0;
}

void ntrip_compat_listener::Auth_Verify_Cb(void *arg, AuthReply *reply)
{
    auto ctx = static_cast<std::pair<ntrip_compat_listener *, json> *>(arg);

    auto svr = ctx->first;
    auto req = ctx->second;

    if (reply->type == AUTH_REPLY_OK)
    {
        QUEUE::Push(req);
    }
    else
    {
    }
}

std::string ntrip_compat_listener::get_conncet_key(bufferevent *bev)
{
    return util_cal_connect_key(bufferevent_getfd(bev));
}

json ntrip_compat_listener::decode_bufferevent_req(bufferevent *bev)
{
    /*
        mount_point
        mount_para
        mount_group
        mount_info          STR STR: ;;;0;;;;;;0;0;;;N;N;
        user_name           Authorization
        user_pwd            Authorization
        user_baseID         Authorization
        user_agent          User-Agent/Source-Agent
        ntrip_version       Ntrip-Version
        ntrip_gga           Ntrip-GGA
        http_chunked        Transfer-Encoding
        http_host           Host

    */
    json info;
    info["mount_point"] = "none";
    info["mount_para"] = "none";
    info["mount_group"] = "common";
    info["mount_info"] = "none";
    info["http_host"] = "none";
    info["http_chunked"] = "unchunked";
    info["user_agent"] = "unknown";
    info["ntrip_version"] = "none";
    info["ntrip_gga"] = "none";
    info["user_baseID "] = "none";
    info["user_name"] = "none";
    info["user_pwd"] = "none";

    evbuffer *evbuf = bufferevent_get_input(bev);
    json item;

    size_t headerlen = 0;
    char *header;
    while (header = evbuffer_readln(evbuf, &headerlen, EVBUFFER_EOL_CRLF))
    {
        if (headerlen > 1024)
        {
            break;
        }
        std::string key_value = header;
        spdlog::debug("[{}:{}]: header line info: {}", __class__, __func__, key_value);

        if (key_value.size() == 0)
        {
            free(header);
            break;
        }
        int x = key_value.find(":");
        if (x == key_value.npos)
        {
            spdlog::warn("[{}:{}]: decode key value fail ,item:", __class__, __func__, key_value);
            break;
        }
        item[key_value.substr(0, x)] = key_value.substr(x + 2);

        free(header);
    }

    if (item["Host"].is_string())
    {
        info["http_host"] = item["Host"];
    }
    if (item["Transfer-Encoding"].is_string())
    {
        info["http_chunked"] = item["Transfer-Encoding"];
    }
    if (item["User-Agent"].is_string())
    {
        info["user_agent"] = item["User-Agent"];
    }
    else if (item["Source-Agent"].is_string())
    {
        info["user_agent"] = item["Source-Agent"];
    }
    if (item["STR"].is_string())
    {
        info["mount_info"] = item["STR"];
    }
    if (item["Ntrip-Version"].is_string())
    {
        info["ntrip_version"] = item["Ntrip-Version"];
    }
    if (item["Ntrip-GGA"].is_string())
    {
        info["ntrip_gga"] = item["Ntrip-GGA"];
    }
    if (item["Authorization"].is_string())
    {
        std::string decodeID = decode_basic_authentication(item["Authorization"]);
        info["user_baseID "] = decodeID;
        int x = decodeID.find(":");
        if (x == decodeID.npos)
        {
            spdlog::warn("[{}:{}]: decode Authorization value illegal ,decode base64:", __class__, __func__, decodeID);
        }
        else
        {
            info["user_name"] = decodeID.substr(0, x);
            info["user_pwd"] = decodeID.substr(x + 1);
        }
    }
    return info;
}

std::string ntrip_compat_listener::extract_path(std::string path)
{
    std::string mount, search;
    if (path.find("/") != 0)
    {
        path.insert(0, "/");
    }

    int x = path.find("?");
    if (x == path.npos)
    {
        mount = path.substr(1, x);
    }
    else
    {
        mount = path.substr(1, x);
        search = path.substr(x + 1);
    }
    return mount;
}

std::string ntrip_compat_listener::extract_para(std::string path)
{
    std::string mount, search;
    if (path.find("/") != 0)
    {
        path.insert(0, "/");
    }

    int x = path.find("?");
    if (x == path.npos)
    {
        mount = path.substr(1, x);
    }
    else
    {
        mount = path.substr(1, x);
        search = path.substr(x + 1);
    }
    return search;
}

std::string ntrip_compat_listener::decode_basic_authentication(std::string authentication)
{
    // Basic bnRyaXA6c2VjcmV0
    char auth[256] = {'\0'};

    if (authentication.find("Basic ") != authentication.npos)
    {
        sscanf(authentication.c_str(), "Basic %s", auth);
    }

    return util_base64_decode(auth);
}

int ntrip_compat_listener::erase_and_free_bev(bufferevent *bev)
{
    std::string Connect_Key = util_cal_connect_key(bufferevent_getfd(bev));

    auto con = _connect_map->find(Connect_Key);
    if (con != _connect_map->end())
    {
        _connect_map->erase(con);
    }
    bufferevent_free(bev);
    return 0;
}
