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

int ntrip_caster::init_state_info()
{
    return 0;
}

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

int ntrip_caster::auto_init()
{
    // 初始化event_base

    // 初始化请求处理事件
    _process_event = event_new(_base, -1, EV_PERSIST, Request_Process_Cb, this);
    _queue->add_processer(_process_event);

    return 0;
}

int ntrip_caster::auto_stop()
{
    return 0;
}

int ntrip_caster::compontent_init()
{
    // 根据配置文件，解析并创建初始化任务

    // 必要任务  注意启动的先后顺序-------------------------------------
    // 添加redis连接请求
    json redis_req = _server_config["Reids_Connect_Setting"];
    create_redis_conncet(redis_req);

    // 添加创建data_tansfer
    json transfer_req = _server_config["Data_Transfer_Setting"];
    create_data_transfer(transfer_req);

    // 创建client_source
    json source_req = _server_config["Source_Setting"];
    create_client_source(source_req);

    // 添加listener请求
    json listener_req = _server_config["Ntrip_Listener_Setting"];
    create_ntrip_listener(listener_req);

    // 可选任务-------------------------------------

    if (_SYS_Relay_Support | _TRD_Relay_Support)
    {
        // 启动一个 connectorr
        json connector_req;
        create_relay_connector(connector_req);
    }

    if (_SYS_Relay_Support)
    {
        // 添加一个系统转发检测的超时回调
    }

    if (_TRD_Relay_Support)
    {
        json relay_req;
        _queue->push(relay_req, ADD_RELAY_MOUNT_TO_LISTENER);
        _queue->push(relay_req, ADD_RELAY_MOUNT_TO_SOURCELIST);
        // 向listener添加准入请求
    }

    if (_HTTP_Ctrl_Support)
    {
    }

    _queue->active_prrocesser();
    return 0;
}

int ntrip_caster::compontent_stop()
{
    _compat_listener->stop();

    _relay_connetcotr->stop();
    delete _relay_connetcotr;

    _transfer->stop();

    _sourcelist->stop();

    redisAsyncDisconnect(_sub_context);
    redisAsyncFree(_sub_context);
    redisAsyncDisconnect(_pub_context);
    redisAsyncFree(_pub_context);

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

    // if(!_server_config["SMTP_Setting"].is_null())
    // {
    //     _redis_beat=new redis_heart_beat(_server_config["Redis_Heart_Beat"]);

    //     _redis_beat->set_base(_base);
    //     _redis_beat->start();
    // }
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

    // static int i = 0;
    // if (i > 1)
    // {
    //     stop();
    // }
    // else
    // {
    //     i++;
    // }

    // if(_info_smtp)
    //  {
    //      _relay_smtp->update_msg(_info_info);
    //  }

    return 0;
}

int ntrip_caster::start()
{

    // 系统开关参数、变量的初始化
    auto_init();
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

    auto_stop();

    // 关闭所有连接，关闭listener;

    event_base_loopexit(_base, NULL);

    return 0;
}

json ntrip_caster::get_setting()
{
    return json();
}

int ntrip_caster::set_setting(json config)
{
    return 0;
}

int ntrip_caster::create_redis_conncet(json req)
{
    _redis_IP = req["Redis_IP"];
    _redis_port = req["Redis_Port"];
    _redis_Requirepass = req["Redis_Requirepass"];

    // 初始化redis连接
    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, _redis_IP.c_str(), _redis_port);
    struct timeval tv = {0};
    tv.tv_sec = 10;
    options.connect_timeout = &tv;

    _pub_context = redisAsyncConnectWithOptions(&options);
    if (_pub_context->err)
    {
        /* Let *c leak for now... */
        spdlog::error("redis eror: {}", _pub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_pub_context, _base);
    redisAsyncSetConnectCallback(_pub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_pub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_pub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());

    _sub_context = redisAsyncConnectWithOptions(&options);
    if (_sub_context->err)
    {
        /* Let *c leak for now... */
        spdlog::error("redis eror: {}", _sub_context->errstr);
        return 1;
    }

    redisLibeventAttach(_sub_context, _base);
    redisAsyncSetConnectCallback(_sub_context, Redis_Connect_Cb);
    redisAsyncSetDisconnectCallback(_sub_context, Redis_Disconnect_Cb);
    redisAsyncCommand(_sub_context, NULL, NULL, "AUTH %s", _redis_Requirepass.c_str());
    return 0;
}

int ntrip_caster::destroy_redis_conncet(json req)
{
    return 0;
}

int ntrip_caster::reconnect_redis_connect(json req)
{
    return 0;
}

int ntrip_caster::create_ntrip_listener(json req)
{
    // 创建一个listen对象，传入要监听的端口
    // 启动监听
    std::string ntrip_version = req["Listener_Type"];
    if (ntrip_version == "NTRIP1.0/2.0")
    {
        auto *listener = new ntrip_compat_listener(_base, _queue, &_connect_map);
        listener->set_listen_conf(req);
        listener->start();
        _compat_listener = listener;
    }
    // else
    // {
    //     auto *listener = new ntrip_common_listener(_base, _queue, &_connect_map, _pub_context);
    //     listener->set_listen_conf(req);
    //     listener->start();
    //     _ntrip_listener = listener;
    // }
    return 0;
}

int ntrip_caster::destroy_ntrip_listener(json req)
{
    return 0;
}

int ntrip_caster::create_client_source(json req)
{
    _sourcelist = new client_source(req, _base, _queue, _sub_context, _pub_context);
    _sourcelist->start();
    return 0;
}

int ntrip_caster::destroy_client_source(json req)
{
    return 0;
}

int ntrip_caster::create_relay_connector(json req)
{
    auto connector = new ntrip_relay_connector(_base, _queue, &_connect_map, _pub_context);
    _relay_connetcotr = connector;

    // 创建的时候，relay表才会生效，才需要读取配置文件
    _relay_accounts.load_account_file("Relay_Accounts.json");

    return 0;
}

int ntrip_caster::create_data_transfer(json req)
{
    _transfer = new data_transfer(req, _queue, _sub_context, _pub_context);
    _transfer->start();

    return 0;
}

int ntrip_caster::destroy_data_transfer(json req)
{
    return 0;
}

int ntrip_caster::create_client_ntrip(json req)
{
    auto arg = new std::pair<ntrip_caster *, json>(this, req);
    std::string mount_group = req["mount_group"];
    std::string Moint_Point = req["mount_point"];
    redisAsyncCommand(_pub_context, Redis_Callback_for_Data_Transfer_add_sub, static_cast<void *>(arg), "HGET mp_ol_all %s", Moint_Point.c_str());

    return 0;
}

int ntrip_caster::send_souce_list(json req)
{
    // 将该请求，加入到source对象中去，由source来处理对应请求
    std::string connect_key = req["connect_key"];
    auto con = _connect_map.find(connect_key);
    if (con == _connect_map.end())
    {
        spdlog::warn("[{}:{}]: fand connect fail, con not in connect_map", __class__, __func__);
        return 1;
    }

    _sourcelist->send_source_list_to_client(req, con->second);

    //_queue->push_and_active(req, ALREADY_SEND_SOURCELIST_CLOSE_CONNCET);

    return 0;
}

int ntrip_caster::create_client_nearest(json req)
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
        _queue->push_and_active(req, NO_IDEL_RELAY_ACCOUNT_CLOSE_CONNCET);
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
        _queue->push_and_active(req, CREATE_RELAY_CONNECT_FAIL_CLOSE_CONNCET);
        return 0;
    }

    // req["mount_point"] = intel_mount;

    // transfer_add_create_client(req);

    return 0;
}

int ntrip_caster::create_server_ntrip(json req)
{

    auto arg = new std::pair<ntrip_caster *, json>(this, req);
    std::string mount = req["mount_point"];
    redisAsyncCommand(_pub_context, Redis_Callback_for_Create_Ntrip_Server, static_cast<void *>(arg), "HGET mp_ol_all %s", mount.c_str());

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
    auto relay = new server_relay(req, con->second, _queue, _sub_context, _pub_context);
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

    transfer_add_create_client(cli_req);

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

    _transfer->del_sub_client(mount_point, connect_key);

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

    if (req_type == REQUEST_VIRTUAL_LOGIN)
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

int ntrip_caster::mount_not_online_close_connect(json req)
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

    switch (reqtype)
    {
    case ALREADY_SEND_SOURCELIST_CLOSE_CONNCET:
        spdlog::info("[{}]:already send sourcelist close connect. addr:[{}:{}]", __class__, ip, port);
        break;
    case MOUNT_NOT_ONLINE_CLOSE_CONNCET:
        spdlog::info("[{}]:mount point not online close connect. mount:[{}], addr:[{}:{}]", __class__, mount, ip, port);
        break;
    case MOUNT_ALREADY_ONLINE_CLOSE_CONNCET:
        spdlog::info("[{}]:mount point already online close connect. mount:[{}], addr:[{}:{}]", __class__, mount, ip, port);
        break;
    case NO_IDEL_RELAY_ACCOUNT_CLOSE_CONNCET:
        spdlog::info("[{}]:no idel relay account close connect. mount:[{}], addr:[{}:{}]", __class__, mount, ip, port);
        break;
    case CREATE_RELAY_CONNECT_FAIL_CLOSE_CONNCET:
        spdlog::info("[{}]:create relay connect fail close connect. mount:[{}], addr:[{}:{}]", __class__, mount, ip, port);
        break;
    default:
        break;
    }

    bufferevent_free(bev);
    _connect_map.erase(item);

    return 0;
}

int ntrip_caster::request_process(json req)
{
    // 根据请求的类型，执行对应的操作
    int REQ_TYPE = req["req_type"];

    spdlog::debug("[{}:{}]: \n\r {}", __class__, __func__, req.dump(2));

    switch (REQ_TYPE)
    {
    // 开关------------------------------------------
    case ENABLE_SYS_NTRIP_RELAY:
        break;
    case DISABLE_SYS_NTRIP_RELAY:
        break;
    case ENABLE_TRD_NTRIP_RELAY:
        break;
    case DISABLE_TRD_NTRIP_RELAY:
        break;
    case ENABLE_HTTP_SERVER:
        break;
    case DISABLE_HTTP_SERVER:
        break;

    // 一般ntrip请求-------------------------------------
    case REQUEST_SOURCE_LOGIN:
        send_souce_list(req);
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

    // relay服务相关--------------------------------------
    case REQUEST_VIRTUAL_LOGIN:
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
    case ADD_RELAY_MOUNT_TO_SOURCELIST:
        add_relay_mount_to_sourcelist(req);
        break;

    // 关闭现有连接------------------------------------------
    case ALREADY_SEND_SOURCELIST_CLOSE_CONNCET:
        mount_not_online_close_connect(req);
        break;
    case MOUNT_NOT_ONLINE_CLOSE_CONNCET:
        mount_not_online_close_connect(req);
        break;
    case MOUNT_ALREADY_ONLINE_CLOSE_CONNCET:
        mount_not_online_close_connect(req);
        break;
    case NO_IDEL_RELAY_ACCOUNT_CLOSE_CONNCET:
        mount_not_online_close_connect(req);
        break;
    case CREATE_RELAY_CONNECT_FAIL_CLOSE_CONNCET:
        mount_not_online_close_connect(req);
        break;

    // 其他操作-------------------------------------------
    default:
        spdlog::warn("undefined req_type: {}", REQ_TYPE);
        break;
    }
    return 0;
}

int ntrip_caster::transfer_add_create_client(json req)
{
    if (!_transfer->mount_point_exist(req["mount_point"]))
    {
        _transfer->add_pub_server(req["mount_point"]);
    }

    // 创建一个client，加入到表中
    client_ntrip *cli = new client_ntrip(req, _connect_map.find(req["connect_key"])->second, _queue, _sub_context, _pub_context);
    _client_map.insert(std::make_pair(req["connect_key"], cli));

    // 将client加入到transfer中
    _transfer->add_sub_client(req["mount_point"], req["connect_key"], cli);

    // 一切准备就绪，启动client（向用户回应）
    cli->start();

    return 0;
}

int ntrip_caster::close_unsuccess_req_connect(json req)
{
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

int ntrip_caster::add_relay_mount_to_sourcelist(json req)
{
    auto mpt = _relay_accounts.get_usr_mpt();
    for (auto iter : mpt)
    {
        _sourcelist->add_Virtal_Mount(iter);
    }
    return 0;
}

void ntrip_caster::Request_Process_Cb(evutil_socket_t fd, short what, void *arg)
{
    ntrip_caster *svr = static_cast<ntrip_caster *>(arg);

    if (svr->_queue->not_null())
    {
        json req = svr->_queue->poll();
        svr->request_process(req);
    }
    if (svr->_queue->not_null())
    {
        svr->_queue->active_prrocesser();
    }
}

void ntrip_caster::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto *svr = static_cast<ntrip_caster *>(arg);

    svr->periodic_task();
}

void ntrip_caster::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: Connected to Redis Success", __class__);
}

void ntrip_caster::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: redis info: Disconnected Redis", __class__);
}

void ntrip_caster::Redis_Callback_for_Data_Transfer_add_sub(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<std::pair<ntrip_caster *, json> *>(privdata);

    auto svr = arg->first;
    auto req = arg->second;

    std::string mount_point = req["mount_point"];
    std::string connect_key = req["connect_key"];

    auto item = svr->_server_key.find(mount_point);
    if (item == svr->_server_key.end())
    {
        svr->_queue->push_and_active(req, MOUNT_NOT_ONLINE_CLOSE_CONNCET);
    }
    else
    {
        svr->transfer_add_create_client(req);
    }

    delete arg;
}

void ntrip_caster::Redis_Callback_for_Create_Ntrip_Server(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<std::pair<ntrip_caster *, json> *>(privdata);

    auto svr = arg->first;
    auto req = arg->second;

    std::string mount_point = req["mount_point"];
    std::string connect_key = req["connect_key"];
    // if (reply->type != REDIS_REPLY_NIL)
    // {
    // }

    auto item = svr->_server_key.find(mount_point);
    if (item == svr->_server_key.end())
    {
        // 本地没有记录
        auto con = svr->_connect_map.find(connect_key);
        if (con == svr->_connect_map.end())
        {
            spdlog::warn("[{}:{}]: Create_Ntrip_Server fail, con not in connect_map", __class__, __func__);
            return;
        }
        server_ntrip *ntrips = new server_ntrip(svr->_server_config["Server_Setting"], req, con->second, svr->_queue, svr->_sub_context, svr->_pub_context);
        // 加入挂载点表中
        svr->_server_key.insert(std::make_pair(mount_point, connect_key));
        svr->_server_map.insert(std::make_pair(connect_key, ntrips));

        // 一切准备就绪，启动server（向用户回应）
        ntrips->start();
    }
    else //(reply->type == REDIS_REPLY_STRING)
    {
        std::string mount_point = req["mount_point"];
        spdlog::info("[{}]:login a same name mount point [{}], kick out the login connection mount point!", __class__, mount_point);

        svr->_queue->push_and_active(req, MOUNT_ALREADY_ONLINE_CLOSE_CONNCET);
    }

    delete arg;
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
