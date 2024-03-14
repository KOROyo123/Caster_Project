#include "heart_beat.h"

#include <unistd.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/resource.h>

#include "rtklib.h"

#define __class__ "redis_heart_beat"

redis_heart_beat::redis_heart_beat(json conf)
{
    _heart_beat_config = conf;

    _heart_beat_switch = conf["Switch"];

    json Reids_Connect_Setting = conf["Reids_Connect_Setting"];

    _redis_IP = Reids_Connect_Setting["Redis_IP"];
    _redis_port = Reids_Connect_Setting["Redis_Port"];
    _redis_Requirepass = Reids_Connect_Setting["Redis_Requirepass"];

    _timeout_tv.tv_sec = conf["Heart_Beat_Interval"];
    _timeout_tv.tv_usec = 0;

    if (conf["Heart_Beat_Method"] == "Pub")
    {
        _heart_beat_method = Pub;
    }
    else
    {
        _heart_beat_method = Set;
    }

    _heart_beat_key = conf["Heart_Beat_Key"];
    _heart_beat_set_msg = conf["Heart_Beat_Set_MSG"];
    update_out_set(conf["Heart_Beat_Out_MSG"]);

    _online_client = _online_server = 0;
    _start_tm = time(0);
}

redis_heart_beat::~redis_heart_beat()
{
}

int redis_heart_beat::set_base(event_base *base)
{
    _base = base;
    return 0;
}

int redis_heart_beat::update_msg(json state_info)
{
    _online_client = state_info["client_num"];
    _online_server = (int)state_info["server_num"] + (int)state_info["relay_num"];
    return 0;
}

int redis_heart_beat::start()
{
    // 连接redis
    connect_to_redis();
    // 添加定时事件
    _timeout_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_timeout_ev, &_timeout_tv);
    return 0;
}

int redis_heart_beat::stop()
{
    return 0;
}

void redis_heart_beat::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto svr = static_cast<redis_heart_beat *>(arg);

    svr->send_heart_beat();
}

void redis_heart_beat::Redis_Connect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: Connected to Redis Success", __class__);
}

void redis_heart_beat::Redis_Disconnect_Cb(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        spdlog::error("[{}]: redis eror: {}", __class__, c->errstr);
        return;
    }
    spdlog::info("[{}]: redis info: Disconnected Redis", __class__);
}

int redis_heart_beat::connect_to_redis()
{

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

json redis_heart_beat::build_beat_msg()
{

    json beat_msg = _heart_beat_set_msg;

    if (_msg_set.PID)
        beat_msg["PID"] = _msg_info.PID;
    if (_msg_set.gpssecond)
        beat_msg["gpssecond"] = _msg_info.gpssecond;
    if (_msg_set.onlineServer)
        beat_msg["onlineServer"] = _msg_info.onlineServer;
    if (_msg_set.onlineClient)
        beat_msg["onlineClient"] = _msg_info.onlineClient;
    if (_msg_set.onlineTime)
        beat_msg["onlineTime"] = _msg_info.onlineTime;
    if (_msg_set.runningtime)
        beat_msg["runningtime"] = _msg_info.runningtime;
    if (_msg_set.memory)
        beat_msg["memory"] = _msg_info.memory;
    if (_msg_set.IP)
        beat_msg["IP"] = _msg_info.IP;

    return beat_msg;
}

int redis_heart_beat::send_heart_beat()
{
    refresh_beat_info();

    json msg = build_beat_msg();
    std::string sendmsg;

    switch (_heart_beat_method)
    {
    case Pub:
        sendmsg = msg.dump();
        redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH %s %b", _heart_beat_key.c_str(), sendmsg.c_str(), sendmsg.size());
        break;
    case Set:
        sendmsg = msg.dump();
        redisAsyncCommand(_pub_context, NULL, NULL, "SET %s %b", _heart_beat_key.c_str(), sendmsg.c_str(), sendmsg.size());
        redisAsyncCommand(_pub_context, NULL, NULL, "EXPIRE %s %d NX", _heart_beat_key.c_str(), _timeout_tv.tv_sec * 3);
        break;
    case HSet:
        // redisAsyncCommand(_pub_context, NULL, NULL, "HSET %s %b", heart_beat_key.c_str(), sendmsg.c_str(), sendmsg.size());
        break;
    default:
        break;
    }

    return 0;
}

int redis_heart_beat::refresh_beat_info()
{

    _msg_info.PID = getpid();
    _msg_info.gpssecond = getGnsssecond();
    _msg_info.onlineServer = _online_server;
    _msg_info.onlineClient = _online_client;
    _msg_info.onlineTime = time(0) - _start_tm;
    _msg_info.runningtime = time(0) - _start_tm;
    _msg_info.memory = getMemory();
    _msg_info.IP = getIP();

    return 0;
}

int redis_heart_beat::update_out_set(json conf)
{
    _msg_set.PID = conf["PID"];
    _msg_set.gpssecond = conf["gpssecond"];
    _msg_set.onlineServer = conf["onlineServer"];
    _msg_set.onlineClient = conf["onlineClient"];
    _msg_set.onlineTime = conf["onlineTime"];
    _msg_set.runningtime = conf["runningtime"];
    _msg_set.memory = conf["memory"];
    _msg_set.IP = conf["IP"];

    return 0;
}

std::string redis_heart_beat::getIP()
{
    std::string IP;

    struct ifaddrs *ifaddr;
    getifaddrs(&ifaddr); // 获取网络接口信息列表
    for (struct ifaddrs *addr = ifaddr; addr != nullptr; addr = addr->ifa_next)
    {
        int family = addr->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            char ip[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &((sockaddr_in *)addr->ifa_addr)->sin_addr, ip, INET_ADDRSTRLEN);
            // std::cout << "Interface: " << addr->ifa_name << ", IP Address: " << ip << std::endl;

            IP = ip;
        }
    }

    freeifaddrs(ifaddr); // 释放内存资源

    return IP;
}

double redis_heart_beat::getGnsssecond()
{
    auto time = timeget();
    return time2gpst(time, NULL);
}

int redis_heart_beat::getMemory()
{
    struct rusage usage;

    // 调用 getrusage() 函数获取当前进程的资源使用情况
    if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
        // std::cerr << "Failed to retrieve resource usage." << std::endl;
        return 0;
    }

    // 输出程序占用的物理内存大小（单位为字节）
    // std::cout << "Memory used by the program in bytes: " << usage.ru_maxrss * 1024 << std::endl;

    return usage.ru_maxrss;
}
