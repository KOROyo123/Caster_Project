#include "client_source.h"

#include "event2/buffer.h"
#include "event2/bufferevent.h"

#define __class__ "client_source"

client_source::client_source(json req, event_base *base, std::shared_ptr<process_queue> queue, redisAsyncContext *sub_context, redisAsyncContext *pub_context)
{
    _setting = req;
    _base = base;
    _queue = queue;

    _pub_context = pub_context;
    _sub_context = sub_context;

    _source_update_tv.tv_sec = req["Update_Interval"];
    _source_update_tv.tv_usec = 0;
    // 源列表输出选项
    _send_common = req["Common_Mount"]["Visibility"];       // 普通挂载点
    _send_trd_relay = req["Trd_Relay_Mount"]["Visibility"]; // 用户连接的第三方挂载点
    _send_sys_relay = req["SYS_Relay_Mount"]["Visibility"]; // 系统转发的挂载点
    _send_nearest = req["NEAREST_Mount"]["Visibility"];     // NEAREST是否可见//Nearest
    _send_virtual = req["Virtual_Mount"]["Visibility"];     // Virtual是否可见//设置的挂载点
}

client_source::~client_source()
{
}

int client_source::start()
{
    get_online_mount_point();

    build_source_list();
    build_source_table();

    // _source_update_tv.tv_sec = 10;
    // _source_update_tv.tv_usec = 0;
    _source_update_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_source_update_ev, &_source_update_tv);

    _using_delay_close_list = _delay_close_list;
    _delay_close_tv.tv_sec = 1;
    _delay_close_tv.tv_usec = 0;
    _delay_close_ev = event_new(_base, -1, EV_PERSIST, DelayCloseCallback, this);
    event_add(_delay_close_ev, &_delay_close_tv);

    return 0;
}

int client_source::stop()
{
    event_del(_source_update_ev);
    event_del(_delay_close_ev);
    return 0;
}

int client_source::send_source_list_to_client(json req, void *connect_obj)
{
    // 解析req

    bufferevent *bev = static_cast<bufferevent *>(connect_obj);

    // 根据是1.0还是2.0协议，发送不同的头
    if (req["ntrip_version"] == "Ntrip/2.0")
    {
        // bufferevent_write_buffer(bev,_ntrip1_source_table);
        bufferevent_write(bev, _ntrip2_source_table.c_str(), _ntrip2_source_table.size());
    }
    else
    {
        // bufferevent_write_buffer(bev,_ntrip1_source_table);
        bufferevent_write(bev, _ntrip1_source_table.c_str(), _ntrip1_source_table.size());
    }

    // 将req放入延时关闭列表
    _using_delay_close_list->push_back(req);

    return 0;
}

int client_source::add_Virtal_Mount(std::string mount_point)
{
    _support_virtual_mount.insert(mount_point);
    return 0;
}

int client_source::del_Virtal_Mount(std::string mount_point)
{
    _support_virtual_mount.erase(mount_point);
    return 0;
}

void client_source::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto svr = static_cast<client_source *>(arg);

    svr->get_online_mount_point();

    svr->build_source_list();
    svr->build_source_table();
}

void client_source::DelayCloseCallback(evutil_socket_t fd, short events, void *arg)
{
    auto svr = static_cast<client_source *>(arg);
    static bool close_delay_list1 = true;

    close_delay_list1 = !close_delay_list1;

    // 将两次回调之间的source请求存储起来，
    // 切换一下表，将上一次存储起来的连接删掉
    if (close_delay_list1)
    {
        svr->_using_delay_close_list = &svr->_delay_close_list[0];
    }
    else
    {
        svr->_using_delay_close_list = &svr->_delay_close_list[1];
    }

    // 关闭所有等待关闭的连接，具体延迟的时间为1t-2t
    svr->close_delay_close();
    //_queue->push_and_active(req, ALREADY_SEND_SOURCELIST_CLOSE_CONNCET);
}

void client_source::Redis_Callback_Get_Common_List(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<client_source *>(privdata);

    if (!reply)
    {
        return;
    }

    arg->_online_common_mount.clear();
    for (int i = 0; i < reply->elements; i += 2)
    {
        arg->_online_common_mount.insert(reply->element[i]->str);
    }
}

void client_source::Redis_Callback_Get_SYS_Relay_List(redisAsyncContext *c, void *r, void *privdata)
{
    auto reply = static_cast<redisReply *>(r);
    auto arg = static_cast<client_source *>(privdata);

    if (reply->type != REDIS_REPLY_NIL)
    {
        return;
    }
    return;
}

void client_source::Redis_Callback_Get_TRD_Relay_List(redisAsyncContext *c, void *r, void *privdata)
{
}

void client_source::Redis_Callback_Get_All_List(redisAsyncContext *c, void *r, void *privdata)
{
}

void client_source::Redis_Callback_Get_One_info(redisAsyncContext *c, void *r, void *privdata)
{
}

int client_source::get_online_mount_point()
{

    if (_send_common)
    {
        redisAsyncCommand(_pub_context, Redis_Callback_Get_Common_List, this, "HGETALL mp_ol_common");
    }
    if (_send_sys_relay)
    {
        redisAsyncCommand(_pub_context, Redis_Callback_Get_SYS_Relay_List, this, "HGETALL mp_ol_sys");
    }
    if (_send_trd_relay)
    {
        redisAsyncCommand(_pub_context, Redis_Callback_Get_TRD_Relay_List, this, "HGETALL mp_ol_trd");
    }

    return 0;
}

int client_source::build_source_list()
{
    _all_items.clear();

    if (_send_nearest)
    {
        _nearest_items = update_list_item(_support_nearest_mount);
    }
    if (_send_virtual)
    {
        _virtual_items = update_list_item(_support_virtual_mount);
    }
    if (_send_sys_relay)
    {
        _sys_relay_items = update_list_item(_online_sys_relay_mount);
    }
    if (_send_common)
    {
        _common_items = update_list_item(_online_common_mount);
    }
    if (_send_trd_relay)
    {
        _trd_relay_items = update_list_item(_online_trd_relay_mount);
    }

    _all_items = _nearest_items +
                 _virtual_items +
                 _sys_relay_items +
                 _common_items +
                 _trd_relay_items;
    return 0;
}

int client_source::build_source_table()
{
    // evbuffer_add_printf(_ntrip1_source_table,"SOURCETABLE 200 OK\r\n");
    // evbuffer_add_printf(_ntrip1_source_table,"Server: Ntrip ExampleCaster 2.0/1.0\r\n");
    // evbuffer_add_printf(_ntrip1_source_table,"Connection: close\r\n");
    // evbuffer_add_printf(_ntrip1_source_table,"Content-Type: text/plain\r\n");
    // evbuffer_add_printf(_ntrip1_source_table,"Content-Length: %ld\r\n",_all_items.size());
    // evbuffer_add_printf(_ntrip1_source_table,"\r\n");
    // evbuffer_add(_ntrip1_source_table,_all_items.data(),_all_items.size());

    _ntrip1_source_table.clear();
    _ntrip1_source_table += "SOURCETABLE 200 OK\r\n";
    _ntrip1_source_table += "Server: Ntrip ExampleCaster 2.0/1.0\r\n";
    _ntrip1_source_table += "Connection: close\r\n";
    _ntrip1_source_table += "Content-Type: text/plain\r\n";
    _ntrip1_source_table += "Content-Length: " + std::to_string(_all_items.size()) + "\r\n";
    _ntrip1_source_table += "\r\n";
    _ntrip1_source_table += _all_items;

    _ntrip2_source_table.clear();
    _ntrip2_source_table += "HTTP/1.1 200 OK\r\n";
    _ntrip2_source_table += "Ntrip-Version: Ntrip/2.0\r\n\r\n";
    _ntrip2_source_table += "Ntrip-Flag: st_filter,st_auth,st_match,st_strict,rtsp\r\n";
    _ntrip2_source_table += "Server: Ntrip ExampleCaster/2.0\r\n";
    _ntrip2_source_table += "Date: Tue, 01 Jan 2008 14:08:15 GMT\r\n";
    _ntrip2_source_table += "Connection: close\r\n";
    _ntrip2_source_table += "Content-Type: gnss/sourcetable\r\n";
    _ntrip2_source_table += "Content-Length: " + std::to_string(_all_items.size()) + "\r\n";
    _ntrip2_source_table += "\r\n";
    _ntrip2_source_table += _all_items;

    return 0;
}

int client_source::close_delay_close()
{
    for (auto iter : *_using_delay_close_list)
    {
        _queue->push_and_active(iter, ALREADY_SEND_SOURCELIST_CLOSE_CONNCET);
    }

    // 清除表
    _using_delay_close_list->clear();

    return 0;
}

std::string client_source::update_list_item(std::set<std::string> group)
{
    std::string items;
    for (auto iter : group)
    {
        mount_info item;
        auto info = _mount_map.find(iter);
        if (info == _mount_map.end())
        {
            item = build_default_mount_info(iter);
        }
        else
        {
            item = info->second;
        }
        items += build_mount_info_to_string(item);
    }
    return items;
}

std::string client_source::build_mount_info_to_string(mount_info i)
{
    std::string item;

    item = i.STR + ";" +
           i.mountpoint + ";" +
           i.identufier + ";" +
           i.format + ";" +
           i.format_details + ";" +
           i.carrier + ";" +
           i.nav_system + ";" +
           i.network + ";" +
           i.country + ";" +
           i.latitude + ";" +
           i.longitude + ";" +
           i.nmea + ";" +
           i.solution + ";" +
           i.generator + ";" +
           i.compr_encrryp + ";" +
           i.authentication + ";" +
           i.fee + ";" +
           i.bitrate + ";" +
           i.misc + ";" + "\r\n";

    return item;
}

mount_info client_source::build_default_mount_info(std::string mount_point)
{
    // STR;              STR;
    // mountpoint;       KORO996;
    // identufier;       ShangHai;
    // format;           RTCM 3.3;
    // format-details;   1004(5),1074(1),1084(1),1094(1),1124(1)
    // carrier;          2
    // nav-system;       GPS+GLO+GAL+BDS
    // network;          KNT
    // country;          CHN
    // latitude;         36.11
    // longitude;        120.11
    // nmea;             0
    // solution;         0
    // generator;        SN
    // compr-encrryp;    none
    // authentication;   B
    // fee;              N
    // bitrate;          9600
    // misc;             caster.koroyo.xyz:2101/KORO996

    mount_info item = {
        "STR",
        mount_point,
        "unknown",
        "unknown",
        "unknown",
        "0",
        "unknown",
        "unknown",
        "unknown",
        "00.00",
        "000.00",
        "0",
        "0",
        "unknown",
        "unknown",
        "B",
        "N",
        "0000",
        "Not parsed or provided"};

    return item;
}
