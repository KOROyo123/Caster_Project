#include "connect_type.h"
#include <string.h>

ntrip_client::ntrip_client(event_base *svr_base, ntripcenter_list *list, connect_info_t *connect_info)
{

    _svr_base = svr_base;

    _list = list;

    _connect_info = connect_info;
}

ntrip_client::ntrip_client()
{
}

ntrip_client::~ntrip_client()
{
    //printf("ntrip_client 调用了析构函数\n");
}

int ntrip_client::start()
{
    // 检索订阅基站是否在线
    if (find_and_update_sub_mount())
    {
        evbuffer_add_printf(_bev->output, "ERROR - Bad Mountpoint\r\n");

        printf("未找到指定的挂载点:%s\n", _connect_info->LocalMount);
        // 没有可用挂载点
        // 如果不在线
        // 发送信息，
        // 发送消息给主线程，告知需要关闭TCP连接,（避免立即关闭？）

        // 设置延时关闭
        delay_close(100);

        return 1;
    }

    // 如果在线

    //  创建一个buffevent绑定回调函数(readcb和eventcb)
#ifdef NTRIP_MULT_THREAD
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE); //-1表示自动创建fd
#else
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
#endif

    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
    bufferevent_enable(_bev, EV_READ);

    // 将自己添加到用户表中
    add_to_table();
    // 将自己添加到订阅表中
    add_to_subclient_table();

    // 一切准备就绪,回复给连接，告诉连接可以开始传输数据
    if (_connect_info->ntrip2)
    {
        evbuffer_add_printf(_bev->output, "HTTP/1.1 200 OK\r\n\r\n");
    }
    else
    {
        evbuffer_add_printf(_bev->output, "ICY 200 OK\r\n\r\n");
    }

    printf("挂载点 %s 用户：%s 已接入，\n", _connect_info->LocalMount, _connect_info->UserName);


    return 0;
}

int ntrip_client::stop()
{

    // 将自身从基站表中删除（不再接收发送的数据）
    dis_subcribe();

    // 从用户表中删除
    del_to_table();
    // 释放buffevent
    bufferevent_free(_bev);

    printf("挂载点 %s 用户：%s 已下线，\n", _connect_info->LocalMount, _connect_info->UserName);

    return 0;
}

int ntrip_client::delay_close(int millisecond)
{
    // 按理来说应该延时关闭

    // 现在先直接关闭

    _connect_info->Proactive_close_connect_flag = 1;
    evutil_closesocket(_connect_info->fd);
    // 之后会触发eventcb 然后释放掉；

    return 0;
}

int ntrip_client::link_connect()
{
    return 0;
}

int ntrip_client::close_connect()
{

    return 0;
}

int ntrip_client::find_and_update_sub_mount()
{

    return 0;
}

int ntrip_client::add_to_subclient_table()
{

    _list->subclient_list.add_sub_client(_connect_info->LocalMount, _connect_info->UserName, _bev);

    return 0;
}

int ntrip_client::del_from_subclient_table()
{
    _list->subclient_list.del_sub_client(_connect_info->LocalMount, _connect_info->UserName);
    return 0;
}

int ntrip_client::add_to_table()
{
    _list->client_list.add(_connect_info->LocalMount, _connect_info->UserName, _bev);

    return 0;
}

int ntrip_client::del_to_table()
{
    _list->client_list.del(_connect_info->LocalMount, _connect_info->UserName);

    return 0;
}

void ntrip_client::ReadCallback(bufferevent *bev, void *arg)
{
    ntrip_client *con = (ntrip_client *)arg;

    evbuffer *recv_data = evbuffer_new();
    bufferevent_read_buffer(bev, recv_data); // 读出数据

    //

    // 接收到GGA等信息，需要解码 更新本地数据

    //

    evbuffer_free(recv_data); // 释放数据
}

void ntrip_client::EventCallback(bufferevent *bev, short events, void *arg)
{
    ntrip_client *con = (ntrip_client *)arg;

    /*
    BEV_EVENT_READING
    BEV_EVENT_WRITING
    BEV_EVENT_EOF
    BEV_EVENT_ERROR
    BEV_EVENT_TIMEOUT
    BEV_EVENT_CONNECTED
    */

    switch (events)
    {
    case BEV_EVENT_EOF:
        /* code */
        break;
    case BEV_EVENT_ERROR:
        /* code */
        break;
    case BEV_EVENT_TIMEOUT:
        /* code */
        break;
    case BEV_EVENT_CONNECTED:
        /* code */
        break;
    case BEV_EVENT_READING:
        /* code */
        break;
    case BEV_EVENT_WRITING:
        /* code */
        break;
    default:
        break;
    }

    con->stop();
}

void ntrip_client::TimeoutCallback(evutil_socket_t *bev, short events, void *arg)
{
    ntrip_client *con = (ntrip_client *)arg;
    // 定期触发函数

    // 执行一些定期操作，清除buffevent缓冲区？

    // 触发邮件系统？发给消息队列，告知指定账号即将到期
}

void ntrip_client::LicenceCheckCallback(evutil_socket_t *bev, short events, void *arg)
{
    ntrip_client *con = (ntrip_client *)arg;

    // 定时断开程序？

    // 可以根据注册日期，设置一个断开时间，

    // 时间触发，则执行断开程序，这样的话，第二次登陆就登不上了，等于智能化的踢出用户

    // 更智能化一步，在踢出用户前，再次从数据库或其他地方检查一下用户的有效期有没有延长，延长了那就重设，更新对象内存储的info

    // 触发邮件系统？发给消息队列，告知指定账号已经到期
}

int ntrip_client::find_and_subscribe()
{

    if (find_and_update_sub_mount())
    {
        return 1;
        // 没有可用挂载点
    }

    add_to_subclient_table();
    return 0;
}

int ntrip_client::dis_subcribe()
{

    del_from_subclient_table();

    return 0;
}
