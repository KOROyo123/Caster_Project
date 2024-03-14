#include "ntrip_common_listener.h"

#define __class__ "ntrip_common_listener"

ntrip_common_listener::ntrip_common_listener(event_base *base, std::shared_ptr<process_queue> queue, std::unordered_map<std::string, void *> *connect_map, redisAsyncContext *pub_context)
{
    _base = base;
    _queue = queue;
    _connect_map = connect_map;
    _pub_context = pub_context;

    _listener = evhttp_new(base);
}

ntrip_common_listener::~ntrip_common_listener()
{
    evhttp_free(_listener);
}

int ntrip_common_listener::set_listen_conf(json conf)
{
    _server_IP = conf["Server_IP"];
    _listen_port = conf["Listener_Port"];

    // _connect_timeout = conf["Connect_Timeout"];
    // _Server_Login_With_Password = conf["Server_Login_With_Password"];
    // _Client_Login_With_Password = conf["Client_Login_With_Password"];
    // _Nearest_Support = conf["Nearest_Support"];
    // _Virtal_Support = conf["Virtal_Support"];

    return 0;
}

int ntrip_common_listener::start()
{
    // 设置通用回调函数，处理其他路径的请求
    evhttp_set_gencb(_listener, Ntrip_Decode_Request_cb, this);
    evhttp_set_cb(_listener, "/", Ntrip_Source_Request_cb, this);

    if (_Nearest_Support)
    {
        enable_Nearest_Support();
    }

    _handle = evhttp_bind_socket_with_handle(_listener, "0.0.0.0", _listen_port);
    if (!_handle)
    {
        // fprintf(stderr, "couldn't bind to port %d. Exiting.\n", port);
        spdlog::error("ntrip listener: couldn't bind to port {}.", _listen_port);
        return 1;
    }

    spdlog::info("ntrip listener: bind to port {} success, start listen...", _listen_port);

    return 0;
}

int ntrip_common_listener::stop()
{

    spdlog::info("ntrip listener: stop bind port %d , stop listener.", _listen_port);

    return 0;
}

std::string ntrip_common_listener::get_conncet_key(evhttp_request *req)
{
    evhttp_connection *con = evhttp_request_get_connection(req);

    char *addr;
    ev_uint16_t port;

    evhttp_connection_get_peer(con, &addr, &port);

    return util_cal_connect_key(_server_IP.c_str(), _listen_port, addr, port);
}

void ntrip_common_listener::Ntrip_Decode_Request_cb(evhttp_request *req, void *arg)
{
    // 根据请求类型分别处理
    switch (evhttp_request_get_command(req))
    {
    case EVHTTP_REQ_GET:
        Ntrip_Client_Request_cb(req, arg); // 新的用户上线
        break;
    case EVHTTP_REQ_POST:
        Ntrip_Server_Request_cb(req, arg); // 新的挂载点上线
        break;
    default:
        // 其他请求
        ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);
        spdlog::debug("ntrip listener: Accept not allowed request");
        svr->decode_evhttp_req(req);
        evhttp_send_error(req, 405, "Method Not Allowed");
        break;
    }
}

int ntrip_common_listener::enable_Nearest_Support()
{
    _Nearest_Support = true;
    evhttp_set_cb(_listener, "/NEAREST", Ntrip_Nearest_Request_cb, this);
    return 0;
}

int ntrip_common_listener::disable_Nearest_Support()
{
    _Nearest_Support = false;
    evhttp_del_cb(_listener, "/NEAREST");
    return 0;
}

int ntrip_common_listener::enable_Virtal_Support()
{
    _Virtal_Support = true;
    return 0;
}

int ntrip_common_listener::disable_Virtal_Support()
{
    _Virtal_Support = false;
    return 0;
}

int ntrip_common_listener::add_Virtal_Mount(std::string mount_point)
{
    if (_Virtal_Support)
    {
        evhttp_set_cb(_listener, mount_point.c_str(), Ntrip_Virtal_Request_cb, NULL);
        return 0;
    }

    return 1;
}

int ntrip_common_listener::del_Virtal_Mount(std::string mount_point)
{
    evhttp_del_cb(_listener, mount_point.c_str());
    return 0;
}

int ntrip_common_listener::redis_Info_Verify(json req)
{

    std::string key = req["connect_key"];
    _req_map.insert(std::make_pair(key, req));

    redisAsyncCommand(_pub_context, Redis_Verify_Callback, this, "");

    return 0;
}

int ntrip_common_listener::redis_Info_Record(json req)
{
    std::string key = req["connect_key"];
    _req_map.insert(std::make_pair(key, req));

    std::string onlinetime = util_get_date_time();
    std::string offlinetime = util_get_space_time();

    int req_type = req["req_type"];
    std::string user = req["user"];
    std::string pwd = req["pwd"];

    auto arg = new std::pair<ntrip_common_listener *, std::string>(this, key);

    redisAsyncCommand(_pub_context, Redis_Record_Callback, static_cast<void *>(arg), "HSET vs_%s type %d user %s pwd %s act_tm %b die_tm %b", key.c_str(), req_type, user.c_str(), pwd.c_str(), onlinetime.c_str(), onlinetime.size(), offlinetime.c_str(), offlinetime.size());
    redisAsyncCommand(arg->first->_pub_context, NULL, NULL, "EXPIRE vs_%s %d NX", arg->second.c_str(), 7200);

    return 0;
}

std::string ntrip_common_listener::decode_basic_authentication(std::string authentication)
{
    // Basic bnRyaXA6c2VjcmV0
    char auth[256] = {'\0'};

    if (authentication.find("Basic ") != authentication.npos)
    {
        sscanf(authentication.c_str(), "Basic %s", auth);
    }

    return util_base64_decode(auth);
}

void ntrip_common_listener::Ntrip_Client_Request_cb(struct evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
        evhttp_send_error(req, 405, "Method Not Allowed");
        return;
    }
    ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);

    svr->process_req(req, REQUEST_CLIENT_LOGIN);
}
void ntrip_common_listener::Ntrip_Virtal_Request_cb(struct evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
        evhttp_send_error(req, 405, "Method Not Allowed");
        return;
    }
    ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);

    svr->process_req(req, REQUEST_VIRTUAL_LOGIN);
}

void ntrip_common_listener::Ntrip_Nearest_Request_cb(evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
        evhttp_send_error(req, 405, "Method Not Allowed");
        return;
    }
    ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);

    svr->process_req(req, REQUEST_NEAREST_LOGIN);
}

void ntrip_common_listener::Ntrip_Server_Request_cb(struct evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
        evhttp_send_error(req, 405, "Method Not Allowed");
        return;
    }
    ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);

    svr->process_req(req, REQUEST_NTRIP_SERVER);
}

void ntrip_common_listener::Ntrip_Source_Request_cb(struct evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
        evhttp_send_error(req, 405, "Method Not Allowed");
        return;
    }
    ntrip_common_listener *svr = static_cast<ntrip_common_listener *>(arg);

    svr->process_req(req, REQUEST_SOURCE_LOGIN);
}

void ntrip_common_listener::Redis_Verify_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<std::pair<ntrip_common_listener *, std::string> *>(privdata);
    auto listener = arg->first;
    auto key = arg->second;
    auto req = listener->_req_map.find(key)->second;
    listener->_req_map.erase(key);
    // 返回用户的密码，有效期，以及属于的分组

    req["user_group"] = "common";

    // listener->_queue->push_and_active(item);
}

void ntrip_common_listener::Redis_Record_Callback(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<std::pair<ntrip_common_listener *, std::string> *>(privdata);
    auto listener = arg->first;
    auto key = arg->second;
    auto req = listener->_req_map.find(key)->second;
    listener->_req_map.erase(key);
    // listener->_queue->push_and_active();

    // redisAsyncCommand(arg->first->_pub_context,NULL,NULL,"EXPIRE %s %d NX",arg->second.c_str(),3600);

    // 设置为普通组
    req["mount_group"] = "common";

    listener->_queue->push_and_active(req, req["req_type"]);

    delete arg;
}

void ntrip_common_listener::Redis_Callback1(redisAsyncContext *c, void *r, void *privdata)
{
    printf("callback1\r\n");
}

void ntrip_common_listener::Redis_Callback2(redisAsyncContext *c, void *r, void *privdata)
{
    printf("callback2\r\n");
}

int ntrip_common_listener::process_req(evhttp_request *req, int req_type)
{
    json item;

    // 解析头
    item = decode_evhttp_req(req);

    // 构建连接基础信息
    std::string connect_key = get_conncet_key(req);

    item["req_type"] = req_type;
    item["connect_key"] = connect_key;
    item["carrier_type"] = CARRIER_TYPE_EVHTTP_REQUEST;

    _connect_map->insert(std::make_pair(connect_key, req));
    _req_map.insert(std::make_pair(connect_key, item));

    login_authentication(item);

    return 0;
}

json ntrip_common_listener::decode_evhttp_req(evhttp_request *req)
{
    // 必含项
    //"req_type":
    //"connect_key":
    //"carrier_type":
    //"mount_point":
    // Ntrip-Version: Ntrip/2.0
    //"user":
    //"pwd":

    // 可选项
    // header中的内容

    json item;

    // 获取请求的挂载点
    item["mount_point"] = evhttp_request_get_uri(req);
    spdlog::debug("ntrip listener: req path: {} ", item["mount_point"]);

    // 解析http header的头
    auto headers = evhttp_request_get_input_headers(req);
    // 依次读取header
    for (auto header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        // header->key, header->value
        spdlog::debug("ntrip listener: header  : {} : {} ", header->key, header->value);
        item[header->key] = header->value;
    }

    if (item["Ntrip-Version"].is_null())
    {
        item["Ntrip-Version"] = "Ntrip/1.0";
    }
    else
    {
        // 对Ntrip-Vesion进行一下验证？
    }

    // 从header中获取用户名和密码
    if (item["Authorization"].is_null())
    {
        item["user"] = "NULL";
        item["pwd"] = "NULL";
    }
    else
    {
        std::string decodeID = decode_basic_authentication(item["Authorization"]);

        int x = decodeID.find(":");
        if (x == decodeID.npos)
        {
            item["user"] = decodeID.c_str();
            item["pwd"] = decodeID.c_str();
        }
        else
        {
            item["user"] = decodeID.substr(0, x);
            item["pwd"] = decodeID.substr(x + 1);
        }
    }

    // // 扩充信息
    // item["mount_point"] = '/';
    // item["Host"] = "ntrip.example.com";
    // item["Ntrip-Version"] = "NTRIP2.0/NTRIP1.0";
    // item["User-Agent"] = "Ntrip ExampleServer/2.0";
    // item["Authorization"] = "bnRyaXA6c2VjcmV0";

    // std::string decodeID = util_base64_decode(item["Authorization"]);
    // int x = decodeID.find(":");
    // item["user"] = decodeID.substr(0, x);
    // item["pwd"] = decodeID.substr(x + 1);

    // 将信息记录到map，用于后续调用

    return item;
}

int ntrip_common_listener::login_authentication(json item)
{

    if (_Server_Login_With_Password)
    {
        // 需要验证，
        redis_Info_Verify(item);
    }
    else
    {
        // 不需要验证，添加记录
        redis_Info_Record(item);
    }

    return 0;
}
