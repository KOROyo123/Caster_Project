#include "server_relay.h"
#include "knt/knt.h"
#define __class__ "server_relay"

server_relay::server_relay(json req, bufferevent* bev)
{
    _info = req;

    _bev = bev;

    int fd = bufferevent_getfd(_bev);
    _ip = util_get_user_ip(fd);
    _port = util_get_user_port(fd);

    if (req["ntrip_version"] == "Ntrip/2.0")
    {
        _NtripVersion2 = true;
        _transfer_with_chunked=true;
    }

    _evbuf = evbuffer_new();

    _connect_key = _info["connect_key"];
    _mount_point = _info["mount_point"];
    _mount_group = _info["mount_group"];

    _req_user_name = _info["origin_req"]["user_name"];
    _req_connect_key = _info["origin_req"]["connect_key"];
}

server_relay::~server_relay()
{
    // redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH mp_offline %s ", _publish_mount.c_str());
    // redisAsyncCommand(_pub_context, NULL, NULL, "HDEL mp_ol_all %s ", _publish_mount.c_str());
    // redisAsyncCommand(_pub_context, NULL, NULL, "HDEL mp_ol_%s %s ", _mount_group.c_str(), _publish_mount.c_str());


    bufferevent_free(_bev);

    evbuffer_free(_evbuf);

    spdlog::info("Mount Info: mount [{}] is offline, addr:[{}:{}]", _mount_point, _ip, _port);
}

int server_relay::start()
{

    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
    bufferevent_enable(_bev, EV_READ | EV_WRITE);

    CASTER::Send_Common_Base_Online_Msg(_mount_point.c_str(), NULL, _connect_key.c_str());

    // redisAsyncCommand(_sub_context, Redis_Recv_Callback, static_cast<void *>(this), "SUBSCRIBE CSTR_%s", _req_user_name.c_str());

    spdlog::info("Mount Info: mount [{}] is online, addr:[{}:{}]", _mount_point, _ip, _port);

    return 0;
}

int server_relay::stop()
{
    bufferevent_disable(_bev, EV_READ);

    // redisAsyncCommand(_sub_context, NULL, NULL, "UNSUBSCRIBE CSTR_%s", _req_user_name.c_str());

    // 向xx发送销毁请求
    json close_req;
    close_req["origin_req"] = _info;
    QUEUE::Push(close_req, CLOSE_RELAY_SERVER);

    CASTER::Send_Common_Base_Offline_Msg(_mount_point.c_str(), NULL, _connect_key.c_str());

    return 0;
}

void server_relay::ReadCallback(bufferevent *bev, void *arg)
{
    auto svr = static_cast<server_relay *>(arg);

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

void server_relay::EventCallback(bufferevent *bev, short events, void *arg)
{
    auto svr = static_cast<server_relay *>(arg);
    svr->stop();
}

int server_relay::publish_data_from_chunck()
{
    if (_chuncked_size == 0)
    {
        // 先读取一行
        char *chunck_head_data;
        size_t chunck_head_size;
        chunck_head_data = evbuffer_readln(_evbuf, &chunck_head_size, EVBUFFER_EOL_CRLF);

        if (!chunck_head_data)
        {
            spdlog::warn("[{}:{}: chunked data error,close connect! mount:[{}], addr:[{}:{}]", __class__, __func__, _mount_point, _ip, _port);
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

        // redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH STR_%s %b", _publish_mount.c_str(), data, _chuncked_size);

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

int server_relay::publish_data_from_evbuf()
{
    size_t length = evbuffer_get_length(_evbuf);

    unsigned char *data = new unsigned char[length + 1];
    data[length] = '\0';

    evbuffer_remove(_evbuf, data, length);

    // redisAsyncCommand(_pub_context, NULL, NULL, "PUBLISH STR_%s %b", _publish_mount.c_str(), data, length);

    // redisFreeCommand

    delete[] data;

    return 0;
}

// void server_relay::Redis_Recv_Callback(redisAsyncContext *c, void *r, void *privdata)
// {
//     auto reply = static_cast<redisReply *>(r);
//     auto svr = static_cast<server_relay *>(privdata);

//     evbuffer *evbuf = bufferevent_get_output(svr->_bev);

//     if (reply->element[2]->type == REDIS_REPLY_INTEGER)
//     {
//         std::string msg;
//         msg = reply->element[0]->str;
//         if (msg == "unsubscribe")
//         {
//             svr->send_del_req();
//         }
//     }
//     else
//     {
//         evbuffer_add(evbuf, reply->element[2]->str, reply->element[2]->len);
//     }

//     // for (int i = 0; i < reply->elements; i++)
//     // {
//     //     auto ele = reply->element[i];

//     //     switch (ele->type)
//     //     {
//     //     case REDIS_REPLY_STRING:
//     //         std::cout << ele->str << std::endl;
//     //         break;

//     //     case REDIS_REPLY_DOUBLE:
//     //         break;

//     //     case REDIS_REPLY_INTEGER:
//     //         std::cout << ele->integer << std::endl;
//     //         break;

//     //     default:
//     //         break;
//     //     }
//     // }

//     // 将接收到的数据，转发给挂载点
// }

// void server_relay::Redis_Unsub_Callback(redisAsyncContext *c, void *r, void *privdata)
// {
//     auto reply = static_cast<redisReply *>(r);
//     auto svr = static_cast<server_relay *>(privdata);

//     // 向xx发送销毁请求
//     json close_req;
//     close_req["origin_req"] = svr->_info;
//     svr->QUEUE::Push(close_req, CLOSE_RELAY_SERVER);

//     spdlog::info("Mount Info: mount [{}] is offline!", svr->_publish_mount);
// }

int server_relay::send_del_req()
{
    // 向xx发送销毁请求
    json close_req;
    close_req["origin_req"] = _info;
    QUEUE::Push(close_req, CLOSE_RELAY_SERVER);

    return 0;
}
