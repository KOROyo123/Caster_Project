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

int ntrip_caster::update_state_info()
{

    // connect_num
    // client_num
    // server_num
    // relay_num

    _state_info["connect_num"] = _connect_map.size();
    _state_info["client_num"] = _client_map.size();
    _state_info["server_num"] = _server_map.size();
    _state_info["relay_num"] = _relays_map.size();

    return 0;
}

ntrip_caster::ntrip_caster(json cfg)
{
    _server_config = cfg;

    std::string dump_conf = _server_config.dump(4);

    spdlog::debug("load conf info:\n{}", dump_conf);

    _base = event_base_new();

    _SYS_Relay_Support = _server_config["Function_Switch"]["SYS_Relay_Support"];
    _TRD_Relay_Support = _server_config["Function_Switch"]["TRD_Relay_Support"];
    _HTTP_Ctrl_Support = _server_config["Function_Switch"]["HTTP_Ctrl_Support"];
}

ntrip_caster::~ntrip_caster()
{
}

int ntrip_caster::compontent_init()
{
    // 根据配置文件，解析并创建初始化任务

    // 必要任务  注意启动的先后顺序-------------------------------------
    // 初始化请求处理队列
    _process_event = event_new(_base, -1, EV_PERSIST, Request_Process_Cb, this);
    QUEUE::Init(_process_event);

    // 初始化Caster数据分发核心：当前采用的是Redis，后续开发支持脱离redis运行
    json redis_req = _server_config["Reids_Connect_Setting"];
    CASTER::Init(redis_req.dump().c_str(), _base);

    // // 添加创建data_tansfer
    // json transfer_req = _server_config["Data_Transfer_Setting"];
    // _data_transfer = new data_transfer(transfer_req);
    // _data_transfer->start();

    // // 创建client_source
    // json source_req = _server_config["Source_Setting"];
    // _source_transfer = new source_transfer(source_req, _base);
    // _source_transfer->start();

    // 创建listener请求
    json listener_req = _server_config["Ntrip_Listener_Setting"];
    _compat_listener = new ntrip_compat_listener(_base, &_connect_map);
    _compat_listener->set_listen_conf(listener_req);
    _compat_listener->start();

    // 可选任务-------------------------------------

    // 创建一个relay_connectorr
    _relay_connetcotr = new ntrip_relay_connector(_base, &_connect_map);
    // relay表读取配置文件
    _relay_accounts.load_account_file("Relay_Accounts.json");
    // }

    if (_SYS_Relay_Support)
    {
        // 添加一个系统转发检测的超时回调
    }

    if (_TRD_Relay_Support)
    {
        json relay_req;
        relay_req["req_type"] = ADD_RELAY_MOUNT_TO_LISTENER;
        QUEUE::Push(relay_req);
        relay_req["req_type"] = ADD_RELAY_MOUNT_TO_SOURCELIST;
        QUEUE::Push(relay_req);
        // 向listener添加准入请求
    }

    return 0;
}

int ntrip_caster::compontent_stop()
{
    _compat_listener->stop();
    delete _compat_listener;

    _relay_connetcotr->stop();
    delete _relay_connetcotr;

    // _data_transfer->stop();
    // delete _data_transfer;

    // _source_transfer->stop();
    // delete _source_transfer;

    CASTER::Free();

    return 0;
}

int ntrip_caster::extra_init()
{
    if (!_server_config["Redis_Heart_Beat"].is_null())
    {
        _redis_beat = new redis_heart_beat(_server_config["Redis_Heart_Beat"]);
        _redis_beat->set_base(_base);
        _redis_beat->start();
    }
    return 0;
}

int ntrip_caster::extra_stop()
{
    if (!_server_config["Redis_Heart_Beat"].is_null())
    {
        _redis_beat->stop();
        delete _redis_beat;
    }

    return 0;
}

int ntrip_caster::periodic_task()
{
    // 定时任务
    spdlog::info("[Service Statistic]: Connection: {}, Online Server: {}, Online Client: {} , Use Memory: {} KB.", _connect_map.size(), _server_map.size(), _client_map.size(), util_get_use_memory());

    malloc_trim(0); // 尝试归还、释放内存

    // 更新记录的状态信息
    update_state_info();

    // 如果配置了redis心跳
    if (_redis_beat)
    {
        _redis_beat->update_msg(_state_info);
    }

    return 0;
}

int ntrip_caster::start()
{
    // 核心模块初始化（核心业务）
    compontent_init();
    // 附加模块初始化（不影响核心业务）
    extra_init();

    // 添加定时事件
    _timeout_tv.tv_sec = 10;
    _timeout_tv.tv_usec = 1;
    _timeout_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_timeout_ev, &_timeout_tv);

    // 启动event_base处理线程
    start_server_thread();

    return 0;
}

int ntrip_caster::stop()
{
    event_del(_timeout_ev);

    extra_stop();

    compontent_stop();

    // 关闭所有连接，关闭listener;

    event_base_loopexit(_base, NULL);

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
    // source->set_source_list(_source_transfer->get_souce_list());
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
        // obj->second->~client_ntrip();
        delete obj->second;
        _source_map.erase(obj);
    }
    return 0;
}

int ntrip_caster::create_client_ntrip(json req)
{
    std::string mount_point = req["mount_point"];
    auto arg = new std::pair<ntrip_caster *, json>(this, req); //
    CASTER::Check_Base_Station_is_Online(mount_point.c_str(), Client_Check_Mount_Point_Callback, arg);
    return 0;
}

int ntrip_caster::create_client_nearest(json req)
{
    // 要结合GEO功能开发
    return 0;
}

int ntrip_caster::create_client_virtual(json req)
{
    return 0;
}

int ntrip_caster::create_relay_connect(json req)
{
    std::string req_mount = req["mount_point"];
    std::string account = _relay_accounts.find_trd_idel_account(req_mount);

    if (account.empty())
    {
        // 没有可用账号
        req["req_type"] = NO_IDEL_RELAY_ACCOUNT_CLOSE_CONNCET;
        QUEUE::Push(req);
        return 0;
    }

    json info = _relay_accounts.get_trd_account_info(account, req_mount);
    info["usr_pwd"] = account;
    info["origin_req"] = req;

    std::string intel_mount = _relay_connetcotr->create_new_connection(info);
    if (intel_mount.empty())
    {
        // 连接建立失败
        // 归还账号
        _relay_accounts.give_back_usr_account(account);
        // 关闭连接
        req["req_type"] = CREATE_RELAY_CONNECT_FAIL_CLOSE_CONNCET;
        QUEUE::Push(req);
        return 0;
    }
    else
    {
        // 先直接建立一个client
    }

    return 0;
}

int ntrip_caster::create_server_ntrip(json req)
{
    std::string mount_point = req["mount_point"];
    auto arg = new std::pair<ntrip_caster *, json>(this, req);
    CASTER::Check_Base_Station_is_Online(mount_point.c_str(), Server_Check_Mount_Point_Callback, arg);
    return 0;
}

int ntrip_caster::create_server_relay(json req)
{
    std::string connect_key = req["connect_key"];
    std::string mount_point = req["mount_point"];

    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
        spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
        return 1;
    }
    auto relay = new server_relay(req, con->second);
    // 加入挂载点表中
    _relays_key.insert(std::make_pair(mount_point, connect_key));
    _relays_map.insert(std::make_pair(connect_key, relay));

    relay->start();

    if (req["origin_req"].is_null())
    {
        return 0;
    }

    json cli_req;
    cli_req = req["origin_req"];
    cli_req["mount_point"] = mount_point;

    // transfer_add_create_client(cli_req);

    return 0;
}

int ntrip_caster::close_server_relay(json req)
{
    json origin_req = req["origin_req"];
    std::string connect_key = origin_req["connect_key"];
    std::string usr_pwd = origin_req["usr_pwd"];

    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
    }
    else
    {
        _connect_map.erase(con);
    }

    auto obj = _relays_map.find(connect_key);
    if (obj == _relays_map.end())
    {
    }
    else
    {
        delete obj->second;
        // obj->second->~server_relay();
        _relays_map.erase(obj);
    }

    // 返还账号
    _relay_accounts.give_back_usr_account(usr_pwd);

    return 0;
}

int ntrip_caster::close_realy_req_connection(json req)
{
    // 关闭连接，找到请求者
    auto relay_req = req["origin_req"];
    std::string usr_pwd = relay_req["usr_pwd"];

    _relay_accounts.give_back_usr_account(usr_pwd);
    if (relay_req["origin_req"].is_null())
    {
        // 没有请求者，是系统创建的请求
        return 0;
    }
    auto cli_req = relay_req["origin_req"];
    std::string connectkey = cli_req["connect_key"];

    auto con = _connect_map.find(connectkey);
    if (con != _connect_map.end())
    {
        bufferevent_free(static_cast<bufferevent *>(con->second));
        _connect_map.erase(con);
    }

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

    // _data_transfer->del_sub_client(mount_point, connect_key);

    auto obj = _client_map.find(connect_key);
    if (obj == _client_map.end())
    {
    }
    else
    {
        // obj->second->~client_ntrip();
        delete obj->second;
        _client_map.erase(obj);
    }

    if (req_type == REQUEST_RELAY_LOGIN)
    {
        // 要把基站也下线
        auto relay_key = _relays_key.find(mount_point);
        if (relay_key != _relays_key.end())
        {
            if (connect_key == relay_key->second)
            {
                _relays_key.erase(mount_point);
            }
        }

        auto relay = _relays_map.find(connect_key);
        if (relay != _relays_map.end())
        {
            relay->second->stop();
        }
    }

    return 0;
}

int ntrip_caster::request_process(json req)
{
    // 根据请求的类型，执行对应的操作
    int REQ_TYPE = req["req_type"];

    // spdlog::debug("[{}:{}]: \n\r {}", __class__, __func__, req.dump(2));
    spdlog::info("[{}:{}]: \n\r {}", __class__, __func__, req.dump(2));

    switch (REQ_TYPE)
    {
    // 一般ntrip请求-------------------------------------
    case REQUEST_SOURCE_LOGIN:
        create_source_ntrip(req);
        break;
    case REQUEST_CLIENT_LOGIN:
        create_client_ntrip(req);
        break;
    case REQUEST_NEAREST_LOGIN:
        create_client_nearest(req);
        break;
    case REQUEST_SERVER_LOGIN:
        create_server_ntrip(req);
        break;
    case CLOSE_NTRIP_CLIENT:
        close_client_ntrip(req);
        break;
    case CLOSE_NTRIP_SERVER:
        close_server_ntrip(req);
        break;
    case CLOSE_NTRIP_SOURCE:
        close_source_ntrip(req);
        break;

    // relay服务相关--------------------------------------
    case REQUEST_RELAY_LOGIN:
        create_relay_connect(req);
        break;
    case CLOSE_RELAY_REQ_CONNECT:
        // 由connector发出的请求，状态为，在连接第三方的过程中，连接失败了
        close_realy_req_connection(req);
        break;
    case CREATE_RELAY_SERVER:
        // 由connector发出的请求，第三方连接成功，创建对应的server和client（如果是client请求的）
        create_server_relay(req);
        break;
    case CLOSE_RELAY_SERVER:
        close_server_relay(req);
        break;
    case ADD_RELAY_MOUNT_TO_LISTENER:
        add_relay_mount_to_listener(req);
        break;
    // case ADD_RELAY_MOUNT_TO_SOURCELIST:
    //     add_relay_mount_to_sourcelist(req);
    //     break;
    // 其他操作-------------------------------------------
    default:
        spdlog::warn("undefined req_type: {}", REQ_TYPE);
        break;
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

int ntrip_caster::add_relay_mount_to_listener(json req)
{
    auto mpt = _relay_accounts.get_usr_mpt();
    for (auto iter : mpt)
    {
        _compat_listener->add_Virtal_Mount(iter);
    }
    return 0;
}

// int ntrip_caster::add_relay_mount_to_sourcelist(json req)
// {
//     auto mpt = _relay_accounts.get_usr_mpt();
//     for (auto iter : mpt)
//     {
//         _source_transfer->add_Virtal_Mount(iter);
//     }
//     return 0;
// }

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

        // 在线才允许上线
        std::string mount_point = req["mount_point"];
        std::string connect_key = req["connect_key"];

        auto con = svr->_connect_map.find(connect_key);
        if (con == svr->_connect_map.end())
        {
            spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
            throw 2; // 找不到连接
        }
        req["Settings"] = svr->_server_config["Client_Setting"];
        client_ntrip *ntripc = new client_ntrip(req, con->second);
 
        svr->_client_map.insert(std::make_pair(connect_key, ntripc));

        // 一切准备就绪，启动server
        ntripc->start();
    }
    catch (int i)
    {
        svr->close_unsuccess_req_connect(req);
    }

    delete ctx;
}

void ntrip_caster::Server_Check_Mount_Point_Callback(const char *request, void *arg, CatserReply *reply)
{
    auto ctx = static_cast<std::pair<ntrip_caster *, json> *>(arg);
    auto svr = ctx->first;
    auto req = ctx->second;

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
        std::string mount_point = req["mount_point"];
        std::string connect_key = req["connect_key"];

        auto con = svr->_connect_map.find(connect_key);
        if (con == svr->_connect_map.end())
        {
            spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
            throw 2; // 找不到连接
        }
        req["Settings"] = svr->_server_config["Server_Setting"];
        server_ntrip *ntrips = new server_ntrip(req, con->second);
        // 加入挂载点表中
        svr->_server_key.insert(std::make_pair(mount_point, connect_key));
        svr->_server_map.insert(std::make_pair(connect_key, ntrips));

        // 一切准备就绪，启动server
        ntrips->start();
    }
    catch (int i)
    {
        svr->close_unsuccess_req_connect(req);
    }

    delete ctx;
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

    // event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

    spdlog::info("Server is runing...");
    event_base_dispatch(base);

    spdlog::warn("Server is stop!"); // 不应当主动发生

    return nullptr;
}
