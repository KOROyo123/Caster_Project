#include "ntrip_caster.h"

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/http.h>

#include <malloc.h> //试图解决linux下（glibc）内存不自动释放问题
// https://blog.csdn.net/kenanxiuji/article/details/48547285
// https://blog.csdn.net/u013259321/article/details/112031002

#define __class__ "ntrip_caster"

ntrip_caster::ntrip_caster(json cfg)
{
    std::string dump_conf = cfg.dump(4);
    spdlog::debug("load conf info:\n{}", dump_conf);

    _service_setting = cfg["Service_Setting"];
    _caster_core_setting = cfg["Core_Setting"];
    _auth_verify_setting = cfg["Auth_Setting"];

    _common_setting = _service_setting["Common_Setting"];

    _listener_setting = _service_setting["Ntrip_Listener"];
    _client_setting = _service_setting["Client_Setting"];
    _server_setting = _service_setting["Server_Setting"];

    _timeout_intv = _common_setting["Timeout_Intv"];
    _output_state = _common_setting["Output_State"];

    _base = event_base_new();

    _timeout_tv.tv_sec = _timeout_intv;
    _timeout_tv.tv_usec = 0;
    _timeout_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
}

ntrip_caster::~ntrip_caster()
{
    event_base_free(_base);
}

int ntrip_caster::start()
{
    // 核心模块初始化（核心业务）
    compontent_init();
    // 添加超时事件
    event_add(_timeout_ev, &_timeout_tv);
    // 启动event_base处理线程
    start_server_thread();

    return 0;
}

int ntrip_caster::stop()
{
    // 删除定超时事件
    event_del(_timeout_ev);
    // 核心模块停止
    compontent_stop();
    // 关闭所有连接，关闭listener;
    event_base_loopexit(_base, NULL);

    return 0;
}

int ntrip_caster::update_state_info()
{
    _state_info["connect_num"] = _connect_map.size();
    _state_info["client_num"] = _client_map.size();
    _state_info["server_num"] = _server_map.size();

    return 0;
}

int ntrip_caster::periodic_task()
{
    if (_output_state) // 输出状态信息
    {
        spdlog::info("[Service Statistic]: Connection: {}, Online Server: {}, Online Client: {} , Use Memory: {} BYTE.", _connect_map.size(), _server_map.size(), _client_map.size(), util_get_use_memory());
    }

    // 更新记录的状态信息
    update_state_info();

#ifdef WIN32

#else
    malloc_trim(0); // 尝试归还、释放内存
#endif

    return 0;
}

int ntrip_caster::compontent_init()
{
    // 初始化请求处理队列
    _process_event = event_new(_base, -1, EV_PERSIST, Request_Process_Cb, this);
    QUEUE::Init(_process_event);

    // 用户验证模块
    AUTH::Init(_auth_verify_setting.dump().c_str(), _base);

    // 初始化Caster数据分发核心：当前采用的是Redis，后续开发支持脱离redis运行
    CASTER::Init(_caster_core_setting.dump().c_str(), _base);
    CASTER::Clear();

    // 创建listener请求
    _compat_listener = new ntrip_compat_listener(_listener_setting, _base, &_connect_map);
    _compat_listener->start();

    return 0;
}

int ntrip_caster::compontent_stop()
{
    _compat_listener->stop();
    delete _compat_listener;

    CASTER::Free();
    return 0;
}

int ntrip_caster::request_process(json req)
{
    // 根据请求的类型，执行对应的操作
    int REQ_TYPE = req["req_type"];

    spdlog::debug("[{}:{}]: \n\r {}", __class__, __func__, req.dump(2));
    spdlog::info("[{}:{}]: REQ_TYPE: {}", __class__, __func__,REQ_TYPE);


    switch (REQ_TYPE)
    {
    // 一般ntrip请求-------------------------------------
    case REQUEST_SOURCE_LOGIN:
        create_source_ntrip(req);
        break;
    case CLOSE_NTRIP_SOURCE:
        close_source_ntrip(req);
        break;
    case REQUEST_CLIENT_LOGIN:
        create_client_ntrip(req);
        break;
    case CLOSE_NTRIP_CLIENT:
        close_client_ntrip(req);
        break;
    case REQUEST_SERVER_LOGIN:
        create_server_ntrip(req);
        break;
    case CLOSE_NTRIP_SERVER:
        close_server_ntrip(req);
        break;
    // 虚拟挂载点  //Nearest/Relay/Cors
    case REQUEST_VIRTUAL_LOGIN:
        create_client_virtual(req);
        break;
    default:
        spdlog::warn("undefined req_type: {}", REQ_TYPE);
        break;
    }
    return 0;
}

int ntrip_caster::create_source_ntrip(json req)
{
    std::string connect_key = req["connect_key"];
    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
        spdlog::warn("[{}:{}]: Create Source_Ntrip fail, con not in connect_map", __class__, __func__);
        return 1;
    }

    auto *source = new source_ntrip(req, con->second);
    _source_map.insert(std::make_pair(connect_key, source));
    source->start();

    return 0;
}

int ntrip_caster::close_source_ntrip(json req)
{
    json origin_req = req["origin_req"];
    std::string connect_key = origin_req["connect_key"];

    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
    }
    else
    {
        _connect_map.erase(con);
    }

    auto obj = _source_map.find(connect_key);
    if (obj == _source_map.end())
    {
    }
    else
    {
        delete obj->second;
        _source_map.erase(obj);
    }
    return 0;
}

int ntrip_caster::create_client_ntrip(json req)
{
    std::string mount_point = req["mount_point"];
    auto arg = new std::pair<ntrip_caster *, json>(this, req); //
    CASTER::Check_Base_Station_is_ONLINE(mount_point.c_str(), Client_Check_Mount_Point_Callback, arg);
    return 0;
}

int ntrip_caster::create_client_virtual(json req)
{
    // 要结合GEO功能开发
    return 0;
}

int ntrip_caster::close_client_ntrip(json req)
{
    json origin_req = req["origin_req"];
    std::string connect_key = origin_req["connect_key"];
    std::string mount_point = origin_req["mount_point"];
    int req_type = origin_req["req_type"];
    auto con = _connect_map.find(connect_key);

    if (con == _connect_map.end())
    {
    }
    else
    {
        _connect_map.erase(con);
    }

    auto obj = _client_map.find(connect_key);
    if (obj == _client_map.end())
    {
    }
    else
    {
        delete obj->second;
        _client_map.erase(obj);
    }
    return 0;
}

int ntrip_caster::create_server_ntrip(json req)
{
    std::string mount_point = req["mount_point"];
    auto arg = new std::pair<ntrip_caster *, json>(this, req);
    CASTER::Check_Base_Station_is_ONLINE(mount_point.c_str(), Server_Check_Mount_Point_Callback, arg);
    return 0;
}

int ntrip_caster::close_server_ntrip(json req)
{
    json origin_req = req["origin_req"];
    std::string connect_key = origin_req["connect_key"];
    std::string mount_point = origin_req["mount_point"];
    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
    }
    else
    {
        _connect_map.erase(con);
    }

    auto obj = _server_map.find(connect_key);
    if (obj == _server_map.end())
    {
    }
    else
    {
        delete obj->second;
        _server_map.erase(obj);
    }

    auto server_key = _server_key.find(mount_point);
    if (server_key != _server_key.end())
    {
        if (connect_key == server_key->second)
        {
            _server_key.erase(mount_point);
        }
    }

    return 0;
}

int ntrip_caster::close_unsuccess_req_connect(json req)
{
    std::string Connect_Key = req["connect_key"];
    int reqtype = req["req_type"];
    std::string mount = req["mount_point"];

    auto item = _connect_map.find(Connect_Key);
    if (item == _connect_map.end())
    {
        spdlog::warn("[{}]:can't find need close connect. mount: [{}] ,connect key: [{}], req type: [{}]", __class__, mount, Connect_Key, reqtype);
        return 1;
    }
    bufferevent *bev = item->second;

    int fd = bufferevent_getfd(bev);
    std::string ip = util_get_user_ip(fd);
    int port = util_get_user_port(fd);

    bufferevent_free(bev);
    _connect_map.erase(item);

    return 0;
}

int ntrip_caster::start_server_thread()
{
    event_base_thread(_base);
    return 0;
}

void *ntrip_caster::event_base_thread(void *arg)
{
    event_base *base = static_cast<event_base *>(arg);
    evthread_make_base_notifiable(base);

    spdlog::info("Server is runing...");
    event_base_dispatch(base);

    spdlog::warn("Server is stop!"); // 不应当主动发生
    return nullptr;
}

void ntrip_caster::Request_Process_Cb(evutil_socket_t fd, short what, void *arg)
{
    ntrip_caster *svr = static_cast<ntrip_caster *>(arg);
    if (QUEUE::Not_Null())
    {
        json req = QUEUE::Pop();
        svr->request_process(req);
    }

    if (QUEUE::Not_Null())
    {
        QUEUE::Active();
    }
}

void ntrip_caster::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto *svr = static_cast<ntrip_caster *>(arg);
    svr->periodic_task();
}

void ntrip_caster::Client_Check_Mount_Point_Callback(const char *request, void *arg, CatserReply *reply)
{
    auto ctx = static_cast<std::pair<ntrip_caster *, json> *>(arg);
    auto svr = ctx->first;
    auto req = ctx->second;

    // 在线才允许上线
    std::string mount_point = req["mount_point"];
    std::string connect_key = req["connect_key"];

    try
    {
        if (reply->type != CASTER_REPLY_INTEGER)
        {
            throw 1; // 异常回复
        }
        if (reply->integer == 0)
        {
            throw 2; // 挂载点不在线
        }

        auto con = svr->_connect_map.find(connect_key);
        if (con == svr->_connect_map.end())
        {
            spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
            throw 2; // 找不到连接
        }
        req["Settings"] = svr->_client_setting;
        client_ntrip *ntripc = new client_ntrip(req, con->second);

        svr->_client_map.insert(std::make_pair(connect_key, ntripc));

        // 一切准备就绪，启动server
        ntripc->start();
    }
    catch (int i)
    {
        spdlog::info("[{}]:Mount [{}] is not online, close client login connect.", __class__, mount_point);
        svr->close_unsuccess_req_connect(req);
    }

    delete ctx;
}

void ntrip_caster::Server_Check_Mount_Point_Callback(const char *request, void *arg, CatserReply *reply)
{
    auto ctx = static_cast<std::pair<ntrip_caster *, json> *>(arg);
    auto svr = ctx->first;
    auto req = ctx->second;

    std::string mount_point = req["mount_point"];
    std::string connect_key = req["connect_key"];

    try
    {
        if (reply->type != CASTER_REPLY_INTEGER)
        {
            throw 1; // 异常回复
        }
        if (reply->integer == 1)
        {
            throw 2; // 已经上线
        }
        // 不在线才允许上线

        auto con = svr->_connect_map.find(connect_key);
        if (con == svr->_connect_map.end())
        {
            spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
            throw 2; // 找不到连接
        }
        req["Settings"] = svr->_server_setting;
        server_ntrip *ntrips = new server_ntrip(req, con->second);
        // 加入挂载点表中
        svr->_server_key.insert(std::make_pair(mount_point, connect_key));
        svr->_server_map.insert(std::make_pair(connect_key, ntrips));

        // 一切准备就绪，启动server
        ntrips->start();
    }
    catch (int i)
    {
        spdlog::info("[{}]:Mount [{}] is already online, close server login connect.", __class__, mount_point);
        svr->close_unsuccess_req_connect(req);
    }

    delete ctx;
}
