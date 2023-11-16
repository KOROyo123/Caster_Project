#include "connect_type.h"

#include <string.h>

ntrip_server::ntrip_server()
{
}

ntrip_server::~ntrip_server()
{
    //printf("ntrip_server 调用了析构函数\n");
}

ntrip_server::ntrip_server(event_base *svr_base, ntripcenter_list *list, connect_info_t *connect_info)
{
    init();

    _svr_base = svr_base;

    _list = list;

    _connect_info = connect_info;
}

int ntrip_server::init()
{

    _svr_base = nullptr;

    _bev = nullptr;

    _timeev = nullptr;
    _timeout.tv_sec = 0;
    _timeout.tv_usec = 0;

    return 0;
}

void ntrip_server::EventCallback(bufferevent *bev, short events, void *arg)
{
    ntrip_server *con = (ntrip_server *)arg;
    // 基站数据流主动/被动断开


    con->stop();

    // 锁定表
    // 依次找到所有用户，设置调用用户的stop方法，停止数据传输

    
}

void ntrip_server::ReadCallback(bufferevent *bev, void *arg)
{
    ntrip_server *con = (ntrip_server *)arg;

    // char data[BUFFEVENT_READ_DATA_SIZE] = {'\0'};
    // bufferevent_read(bev, data, BUFFEVENT_READ_DATA_SIZE);
    // printf("%s", data);
    //  锁定表(读锁)

    // 接收到基站的数据
    evbuffer *recv_data = evbuffer_new();
    bufferevent_read_buffer(bev, recv_data);
    // 分发数据
    con->data_dispatch(recv_data);
    // 解析数据
    con->data_decode(recv_data);
    // 释放数据

    // char *line=evbuffer_readline(recv_data);

    // if(line)
    // {
    //     printf("%s\n",line);
    // }
    
    // free(line);
    
    evbuffer_free(recv_data);

    // 引用到指定桶的bufferevent

    // 数据分发
    //  锁定表
    //  检索所有挂载在该点下的用户，依次将数据写入到对应的bev缓冲区
    //
}

int ntrip_server::start()
{

    // RelayInfo->fd = bufferevent_getfd(bev);
#ifdef NTRIP_MULT_THREAD
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE); //-1表示自动创建fd
#else
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
#endif

    // BEV_OPT_CLOSE_ON_FREE：释放bufferevent自动关闭底层接口
    // BEV_OPT_THREADSAFE：使bufferevent能够在多线程下是安全的

    //  注册事件回调
    bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
    bufferevent_enable(_bev, EV_READ); // TODO:参数意义

    // event_base_dump_events(_svr_base, stdout); // 向控制台输出当前已经绑定在base上的事件

    // 添加到基站表中
    add_to_table();
    // 创建一个buffevent绑定回调函数(readcb和eventcb) （可写绑定sed_base,可读绑定rcv_base)

    // 一切准备就绪,回复给连接，告诉连接可以开始传输数据
    
    if (_connect_info->ntrip2)
    {
        evbuffer_add_printf(_bev->output,"HTTP/1.1 200 OK\r\n\r\n");
    }
    else
    {
        evbuffer_add_printf(_bev->output,"ICY 200 OK\r\n\r\n");
    }

    printf("基站：%s 已上线\n",_connect_info->LocalMount);

    return 0;
}

int ntrip_server::stop()
{

    del_to_table();//只下线自己，不管订阅的基站，  订阅的基站添加一个readcb timeout，如果超过时间没有数据，那就检测是否基站下线了

    bufferevent_free(_bev);
    
    printf("基站：%s 已下线\n",_connect_info->LocalMount);


    //告知NtipCenter 自己下线了


    delete(this);
    return 0;
}

int ntrip_server::set_bev_timeout(int read_timeout_sec, int write_timeout_sec)
{
    if (read_timeout_sec)
    {
        _tv_read->tv_sec = read_timeout_sec;
        _tv_read->tv_usec = 0;
    }
    if (write_timeout_sec)
    {
        _tv_write->tv_sec = read_timeout_sec;
        _tv_write->tv_usec = 0;
    }

    // bufferevent_set_timeouts(_bev,_tv_read,_tv_write);
    return 0;
}

int ntrip_server::add_to_table()
{
    _list->server_list.add(_connect_info->LocalMount, _connect_info->UserName, _bev);
    return 0;
}

int ntrip_server::del_to_table()
{
    _list->server_list.del(_connect_info->LocalMount, _connect_info->UserName);
    return 0;
}

int ntrip_server::data_dispatch(evbuffer *recv_data)
{

    ntrip_subclient_entry *trans_entry;
    evbuffer *trans_evbuf=evbuffer_new();

    trans_entry = _list->subclient_list.find_client_by_submount(_connect_info->LocalMount);

    while (trans_entry)
    {
        evbuffer_add_buffer_reference(trans_evbuf, recv_data);
        bufferevent_write_buffer(trans_entry->bev, trans_evbuf);

        trans_entry = _list->subclient_list.find_next_subclient(trans_entry);
    }

    return 0;
}

int ntrip_server::data_decode(evbuffer *recv_data)
{
    evbuffer *decode_evbuf=evbuffer_new();
    evbuffer_add_buffer_reference(decode_evbuf, recv_data);


    evbuffer_free(decode_evbuf);
    return 0;
}


