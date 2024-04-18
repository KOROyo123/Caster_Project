#define __class__ "redis_msg_internal"
#include "Caster_Core.h"
#include <string>
#include <unordered_map>
#include <set>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class sub_cb_item
{
public:
    CasterCallback cb;
    void *arg;
};

class redis_msg_internal
{
private:
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    event_base *_base;

private:
    //----------本地-------------------
    // 订阅表
    std::unordered_map<std::string, std::unordered_map<std::string, sub_cb_item>> _sub_cb_map; // channel/connect_key/cb_arg

    //----------公共-------------------
    // 活跃频道 HASH(CHANNEL:ACTIVE)
    std::set<std::string> _active_channel;

    // 普通挂载点
    // 挂载点在线 HASH(MOUNT:COMMON:ONLINE)

    // 挂载点信息 HASH(MOUNT:COMMON:INFO) //人工设置

    // 挂载点坐标 HASH(MOUNT:COMMON:GEO)

    // 虚拟参考站挂载点 HASH(MOUNT:VIRTUAL:LIST)







public:
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_msg_internal(json conf, event_base *base);
    ~redis_msg_internal();

    int start();
    int stop();

    int add_sub_cb_item(const char *channel, const char *connect_key, CasterCallback cb, void *arg);
    int del_sub_cb_item(const char *channel, const char *connect_key);

    int check_active_channel();

    // Redis回调
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);

    static void Redis_SUB_Callback(redisAsyncContext *c, void *r, void *privdata);
    static void Redis_ONCE_Callback(redisAsyncContext *c, void *r, void *privdata);

    static void Redis_Get_Hash_Field_Callback(redisAsyncContext *c, void *r, void *privdata);

    static long long get_time_stamp();

private:
    // 源列表输出选项
    bool _send_common = true;     // 普通挂载点
    bool _send_trd_relay = false; // 用户连接的第三方挂载点
    bool _send_sys_relay = false; // 系统转发的挂载点
    bool _send_nearest = false;   // NEAREST是否可见//Nearest
    bool _send_virtual = false;   // Virtual是否可见//设置的挂载点

    std::unordered_map<std::string, mount_info> _mount_map; // Mount_Point // 挂载点名为XXXX-F1A6(虚拟挂载点名-本地连接第三方时采用的端口转为4位16进制)

    std::set<std::string> _online_common_mount;
    std::set<std::string> _online_sys_relay_mount;
    std::set<std::string> _online_trd_relay_mount;
    std::set<std::string> _support_virtual_mount;
    std::set<std::string> _support_nearest_mount;

    std::string _common_items;
    std::string _sys_relay_items;
    std::string _trd_relay_items;
    std::string _virtual_items;
    std::string _nearest_items;

    std::string _source_list_text;

    event *_timeout_ev;
    timeval _timeout_tv;

public:
    std::string get_source_list_text();

private:
    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);

    int update_source_list();
    int build_source_list();

    std::string convert_group_mount_to_string(std::set<std::string> group);
    std::string convert_mount_info_to_string(mount_info item);
    mount_info build_default_mount_info(std::string mount_point);
};
