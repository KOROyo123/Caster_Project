/*
    为什么要单独抽象出一层TCP连接，主要是为了功能细化
    同时也考虑到要取维护连接表，采用每个对象维护自己的所在表的功能
    TCP对象除了根据类型划分创建不同的子对象外，还负责维护自身的TCP表，由自己来主动添加、删除自己（在创建的时候添加表，在连接断开的时候移除表）
    同时也是为了保持表的结构统一，TCP连接表、Server表、Client表等本质来说都是存了一个连接
*/
#include "connect_type.h"

#include <string.h>

#include "knt/base64.h"

int Decode_base64(char *base64, char *username, char *password)
{


    std::string undecode = base64;

    std::string decode;

    decode = util_base64_decode(undecode);

    char user_and_pwd[64];
    strcpy(user_and_pwd, decode.c_str());

    char *p;

    if (!(p = strchr(user_and_pwd, ':')))
    {
        // base64解析有问题
        return 1;
    }

    strncpy(username, user_and_pwd, p - user_and_pwd);
    strcpy(password, p + 1);
    return 0;
}

void tcp_connect::WriteCallback(bufferevent *bev, void *arg)
{
    tcp_connect *con = (tcp_connect *)arg;

    switch (con->_connect_info->type)
    {
    case TCP_CONNECT_TYPE_SERVER_RELAY:
        // send_relay_request()
        break;

    default:
        break;
    }

    bufferevent_disable(bev, EV_WRITE); // 即Write触发一次后就停止，除非被再次enable
}

void tcp_connect::ReadCallback(bufferevent *bev, void *arg)
{
    tcp_connect *con = (tcp_connect *)arg;

    evbuffer *recv_data = evbuffer_new();
    bufferevent_read_buffer(bev, recv_data);

    if (con->login_verify(recv_data)) // 这个函数会读出（并删除）recv_data的数据
    {
        // 登录验证失败
        // con->delay_close(50);
        con->stop(); // 验证失败，断开连接
    }
    else
    {
        con->create_connect_by_type(); // 验证成功，进入下一步
    }

    evbuffer_free(recv_data);
}

void tcp_connect::EventCallback(bufferevent *bev, short events, void *arg)
{
    tcp_connect *con = (tcp_connect *)arg;

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
    case BEV_EVENT_TIMEOUT|BEV_EVENT_READING:
        break;

    default:
        if (con->_connect_info->Proactive_close_connect_flag)
        {
            // 主动断开连接
        }
        else
        {
            // 用户断开的连接
        }

        break;
    }

    // 连接断开
    con->stop();
}

int tcp_connect::start()
{
    // 创建一个绑定在base上的buffevent，并建立socket连接

#ifdef NTRIP_MULT_THREAD
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE); //-1表示自动创建fd
#else
    _bev = bufferevent_socket_new(_svr_base, _connect_info->fd, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
#endif

    // BEV_OPT_CLOSE_ON_FREE：释放bufferevent自动关闭底层接口
    // BEV_OPT_THREADSAFE：使bufferevent能够在多线程下是安全的

    if (_connect_info->type == TCP_CONNECT_TYPE_COMMON_NTRIP)
    {
        //  被动连接，等待验证，注册事件回调
        bufferevent_setcb(_bev, ReadCallback, NULL, EventCallback, this);
        bufferevent_enable(_bev, EV_READ); //
    }
    else if (_connect_info->type == TCP_CONNECT_TYPE_ACTIVE_NTRIP)
    {
        // 如果是主动连接模式，即需要发验证信息给fd，需要多绑定一个写回调
        bufferevent_setcb(_bev, ReadCallback, WriteCallback, EventCallback, this);
        bufferevent_enable(_bev, EV_READ | EV_WRITE); // EV_WRITE应当只触发一次，所以再write_cb里面应当disable EV_WRITE
    }

    _bev_read_timeout.tv_sec = 10;
    _bev_read_timeout.tv_usec = 0;

    bufferevent_set_timeouts(_bev, &_bev_read_timeout, NULL);

    // event_base_dump_events(_svr_base, stdout); // 向控制台输出当前已经绑定在base上的事件

    // 将连接添加到TCP连接表中
    add_to_table();

    printf("TCP连接已接入,套接字:%d \n", _connect_info->fd);

    return 0;
}

/*
    //超时触发函数，也可提前触发
    如果账号密码验证成功，则立即激活，如果指定时间内验证失败，则触发关闭程序，回复超时
*/
void tcp_connect::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{

    // 设置超时函数，在指定时间内没能完成验证，踢出连接（例如一直不发送验证的头）

    // 设置超时函数
    // con->_timeev = event_new(con->_svr_base,-1,EV_TIMEOUT,TimeoutCallback,con);

    // con->_timeout.tv_sec =EVENT_TIMEOUT_SEC;
    // con->_timeout.tv_usec = 0;
    // event_add(con->_timeev, &con->_timeout);

    // 如果验证不成功，给连接发送指定错误报文

    // 问题：如果验证失败，按理来说要发送一个回复消息，如果我直接把要发送的消息写道缓冲区，再直接调用close方法，那么这个消息有没有可能发不出去，buffevent就直接被释放了
    // 所以还需要等待一段时间再关闭
}

void tcp_connect::LicenceExpireCallback(evutil_socket_t fd, short events, void *arg)
{

    // 失效函数触发

    // 更新失效函数
}

void tcp_connect::DelayCloseCallback(evutil_socket_t fd, short what, void *arg)
{
    timeval *tv = (timeval *)arg;

    evutil_closesocket(fd);

    free(tv);
    // 之后会触发eventcb 然后释放掉；
}

int tcp_connect::add_to_table()
{

    _list->tcp_list.add(_connect_info->fd);

    return 0;
}

int tcp_connect::del_to_table()
{

    _list->tcp_list.del(_connect_info->fd);
    return 0;
}

int tcp_connect::check_userID()
{
    // 本地数据库检验

    // 将用户名和密码发送给查询程序，等待回应

    // 传入用户名和密码
    // 返回有效期

    // 更新用户信息，更新用户可用性和有效期

    // 用户名存在，密码校验通过

    // 在在线表中插入一条记录
    // 插入失败（已登录）
    // 查询是否允许重复登录
    // 不允许

    // 允许

    // 插入成功（）
    return 0;
}

int tcp_connect::check_mountPoint()
{

    switch (_connect_info->type)
    {
    case TCP_CONNECT_TYPE_CLIENT_NTRIP:
        if (!_list->relay_account_list.search_Maping_Mount_list(_connect_info->LocalMount))
        {
            _connect_info->type = TCP_CONNECT_TYPE_CLIENT_RELAY;
            break;
        }
        else if (_list->server_list.is_exist(_connect_info->LocalMount) == NULL)
        {
            evbuffer_add_printf(_bev->output, "ERROR - Bad Mountpoint\r\n");
            printf("未找到指定的挂载点:%s\n", _connect_info->LocalMount);
            return 1; // 未找到指定挂载点
        }
        break;
    case TCP_CONNECT_TYPE_SERVER_NTRIP:
        if (_list->server_list.is_exist(_connect_info->LocalMount))
        {
            printf("已有同名挂载点:%s\n", _connect_info->LocalMount);
            return 1; // 对于server来说，如果有新的同名挂载点，那么应当不允许其挤掉之前的连接？
        }
        break;
    case TCP_CONNECT_TYPE_GET_LIST:

        break;
    default:
        // 只有client和server需要检测挂载点，其他的直接通过
        break;
    }

    return 0;
}

int tcp_connect::match_relay_mount()
{
    // 没有检索到
    return 1;

    // 检索到
}

/*
    被动连接模式的构造函数 // 接收一个已经创建的连接，传入fd
    设置了两种种类型的base，但实际上也可以都传同一个base，（主要是考虑如果用户多的话，分工一下能够更好的运用多核性能？）

*/
tcp_connect::tcp_connect(event_base *svr_base, ntripcenter_list *list, bufferevent *base_bev, int connect_type, evutil_socket_t conn_fd)
{
    // 所有成员初始化
    init();

    // 接收参数到类内
    _svr_base = svr_base;

    _list = list;

    _connect_info->type = connect_type;
    _connect_info->fd = conn_fd;

    _base_bev = base_bev;
    // 用evutil_xxxx函数获取 addr和port
}

tcp_connect::tcp_connect(event_base *svr_base, ntripcenter_list *list, bufferevent *base_bev, int connect_type, evutil_socket_t conn_fd, json *relayinfo)
{

    // 所有成员初始化
    init();

    // 接收参数到类内
    _svr_base = svr_base;
    _base_bev = base_bev;

    _list = list;

    _connect_info->type = connect_type;
    _connect_info->fd = conn_fd;
}

// 创建一个空的对象，准备传入连接
tcp_connect::tcp_connect()
{
    init();
}

tcp_connect::~tcp_connect()
{
    // printf("tcp_connect调用了析构函数\n");
}

int tcp_connect::init()
{
    // 指针初始化和分配空间
    _bev = nullptr;

    _timeev = nullptr;
    _timeout.tv_sec = 0;
    _timeout.tv_usec = 0;

    _svr_base = nullptr;

    _list = nullptr;

    _connect_info = new connect_info_t{0};

    _error_type = 0;
    _ntrip_server = nullptr;
    _ntrip_client = nullptr;
    _relay_server = nullptr;
    _relay_client = nullptr;
    _que_client = nullptr;

    return 0;
}

void tcp_connect::stop()
{
    // 在这个阶段调用的close_connect，一般说明这个事件在tcp连接阶段被断开的，因此只本函数需要维护tcp表就够了

    // 如果是主动断开的，如果这个事件刚刚解绑，但是还没有连接到子对象？那么fd关闭了，有没有可能tcp表没有关闭
    // A:一般来说，是先子对象绑定完成后，再解绑当前对象的回调绑定，这样的话，其实是有重叠时间，但是没有间隙时间，理论上应该不会发生，
    // 但是需要保证，子对象断开连接触发后，其需要对其维护的表都进行更新操作

    // 从表中移除
    del_to_table();

    bufferevent_free(_bev);

    printf("TCP连接已断开,套接字:%d \n", _connect_info->fd);

    delete (this);
}

int tcp_connect::delay_close(int millisecond)
{
    // 现在先直接关闭

    _connect_info->Proactive_close_connect_flag = 1;

    timeval *tv = (timeval *)malloc(sizeof(timeval *));

    int msec = millisecond % 1000;

    tv->tv_sec = (millisecond - msec) / 1000;
    tv->tv_usec = msec * 1000;

    event *ev = event_new(_svr_base, _connect_info->fd, 0, DelayCloseCallback, tv);
    event_add(ev, tv);

    // event_active(ev, 0, 0);

    return 0;
}

int tcp_connect::send_relay_request()
{
    evbuffer *send_data = evbuffer_new();

    if (_connect_info->ntrip2)
    {
        evbuffer_add_printf(send_data, "GET %s HTTP/1.0\r\n", _connect_info->ConMount);
        evbuffer_add_printf(send_data, "Host: %s\r\n", _connect_info->host);
        evbuffer_add_printf(send_data, "Ntrip-Version: Ntrip/2.0\r\n");
        evbuffer_add_printf(send_data, "User-Agent: %s/%s\r\n", SOFTWARE_NAME, SOFTWARE_VERSION);
        evbuffer_add_printf(send_data, "Referer: RELAY\r\n");
        evbuffer_add_printf(send_data, "Connection: close\r\n");
        evbuffer_add_printf(send_data, "Authorization: Basic %s\r\n\r\n", _connect_info->Base64UserID);
    }
    else
    {
        evbuffer_add_printf(send_data, "GET /%s HTTP/1.0\r\n", _connect_info->ConMount);
        evbuffer_add_printf(send_data, "User-Agent: %s/%s\r\n", SOFTWARE_NAME, SOFTWARE_VERSION);
        if (_connect_info->Base64UserID)
            evbuffer_add_printf(send_data, "Authorization: Basic %s\r\n\r\n", _connect_info->Base64UserID);
        else
            evbuffer_add_printf(send_data, "Accept: */*\r\nConnection: close\r\n\r\n");
    }

    bufferevent_write_buffer(_bev, send_data);

    evbuffer_free(send_data);
    return 0;
}

/*
    接收到的连接信息，解码和验证，解析成功返回0，解析失败，返回
*/
int tcp_connect::login_verify(evbuffer *recv_data)
{

    // 解析头 填写connect_info信息
    if (decode_header(recv_data))
    {
        //  解析失败，解析失败/解析有问题/不完整        //  关闭连接？  也可等待下次报文
        return 1;
    }
    if (check_userID()) // 利用userID，检索user表调用user表的方法，检索userID是否在表中有记录/如果开启匿名登录，那么也会返回一个1
    {

        return 2;
    }
    if (check_mountPoint()) // 如果是挂载点类型，检测挂载点是否存在（对于Server来说需要不存在，对于Client来说需要存在，对于relay来说，指定挂载点要存在）// 如果是其他类型，直接返回？
    {
        return 3;
    }

    return 0;
}
int tcp_connect::decode_header(evbuffer *recv_data)
{
    char *line;

    int ret = -1; // request 请求错误

    line = evbuffer_readline(recv_data); // 解析请求类型

    if (line == NULL || sizeof(line) > 63)
    {
        // 限制恶意输入可能导致程序出现问题
        // 行长度异常
        return -1;
    }

    if (strstr(line, "GET") != NULL) // 注释行 忽略strstr
    {
        ret = decode_get_request(line) || ntrip_client_header_decode(recv_data);
    }
    else if (strstr(line, "POST")) //
    {
        ret = decode_post_request(line) || ntrip_server_header_decode(recv_data);
    }
    else if (strstr(line, "SOURCE")) //
    {
        ret = decode_source_request(line) || ntrip_server_header_decode(recv_data);
    }
    else if (strstr(line, "HTTP/1.1 200 OK")) //
    {
        ret = ntrip_relay_header_decode(1);
    }
    else if (strstr(line, "ICY 200 OK")) //
    {
        ret = ntrip_relay_header_decode(0);
    }

    // 补全连接信息
    // 设置接入挂载点和映射挂载点相同//其他模式，如NEAREST/RELAY等后续更改挂载点，更改的是本地挂载点
    strncpy(_connect_info->LocalMount, _connect_info->ConMount, sizeof(_connect_info->ConMount));

    free(line);
    return ret;
}

int tcp_connect::decode_get_request(char *request)
{
    // 【GET /xxxxxxx HTTP/1.0】 or 【GET xxxxxx HTTP/1.0】 or【GET / HTTP/1.0】  【GET HTTP/1.0】
    char *p1, *p2;
    if (!(p1 = strchr(request, ' ')))
    {
        // 没有空格 有问题
        return 1;
    }
    if (!(p2 = strchr(p1 + 1, ' ')))
    {
        // 没有空格
        // 判定为获取源列表？
        strcpy(_connect_info->http_version, p1 + 1);

        _connect_info->type = TCP_CONNECT_TYPE_GET_LIST;
        return 0;
    }

    int size = p2 - p1;
    if (p1[1] == '/')
    {
        if (p1[2] == ' ')
        {
            strcpy(_connect_info->http_version, p2 + 1);

            _connect_info->type = TCP_CONNECT_TYPE_GET_LIST;
            return 0;
        }
        // strncmp(_connect_info->ConMount,p1+1,(p2-(p1+1)));
        strncpy(_connect_info->ConMount, p1 + 2, size - 2);
    }
    else
    {
        strncpy(_connect_info->ConMount, p1 + 1, size - 1);
    }
    strcpy(_connect_info->http_version, p2 + 1);

    _connect_info->type = TCP_CONNECT_TYPE_CLIENT_NTRIP;
    return 0;
}

int tcp_connect::decode_source_request(char *request)
{
    /*
    SOURCE asdasda 222
    Source-Agent: NTRIP RTKLIB/2.4.3
    STR: ;;;0;;;;;;0;0;;;N;N;
    */
    // 【SOURCE password 222】 or 【SOURCE password /222】or【SOURCE password /222 HTTP/1.1】
    char *p1, *p2, *p3;
    if (!(p1 = strchr(request, ' ')))
    {
        // 没有空格 有问题
        return 1;
    }
    if (!(p2 = strchr(p1 + 1, ' ')))
    {
        // 没有空格 有问题 直接return
        return 1;
    }
    int size = p2 - p1;

    strncpy(_connect_info->Password, p1 + 1, size - 1);

    if (p3 = strchr(p2 + 1, ' '))
    {
        strcpy(_connect_info->http_version, p3 + 1);

        int size = p3 - p2;
        if (p2[1] == '/')
            strncpy(_connect_info->ConMount, p2 + 2, size - 2);
        else
            strncpy(_connect_info->ConMount, p2 + 1, size - 1);
    }
    else
    {
        strcpy(_connect_info->http_version, "HTTP/1.0");

        if (p2[1] == '/')
            strcpy(_connect_info->ConMount, p2 + 2);
        else
            strcpy(_connect_info->ConMount, p2 + 1);
    }

    // 因为这种登录没有用户名，设置挂载点名为固定值
    strcpy(_connect_info->UserName, "Ntrip_Server_1.0");

    _connect_info->ntrip2 = 0;
    _connect_info->type = TCP_CONNECT_TYPE_SERVER_NTRIP;
    return 0;
}
int tcp_connect::decode_post_request(char *request)
{
    // 【POST /xxxxxxx HTTP/1.0】 or 【POST xxxxxx HTTP/1.0】
    char *p1, *p2;
    if (!(p1 = strchr(request, ' ')))
    {
        // 没有空格 有问题
        return 1;
    }
    if (!(p2 = strchr(p1 + 1, ' ')))
    {
        // 没有空格 有问题
        return 1;
    }

    int size = p2 - p1;
    if (p1[1] == '/')
    {
        // strncmp(_connect_info->ConMount,p1+1,(p2-(p1+1)));
        strncpy(_connect_info->ConMount, p1 + 2, size - 2);
    }
    else
    {
        strncpy(_connect_info->ConMount, p1 + 1, size - 1);
    }
    strcpy(_connect_info->http_version, p2 + 1);

    _connect_info->ntrip2 = 1;
    _connect_info->type = TCP_CONNECT_TYPE_SERVER_NTRIP;
    return 0;
}

int tcp_connect::ntrip_client_header_decode(evbuffer *recv_data)
{

    // GET / HTTP/1.0
    // User-Agent: NTRIP RTKLIB/2.4.3
    // Accept: */*
    // Connection: close

    // GET /asdasd HTTP/1.0
    // User-Agent: NTRIP RTKLIB/2.4.3
    // Authorization: Basic YXNkYXM6c2Rhc2Q=

    // User-Agent: NTRIP RTKLIB/2.4.3
    // Accept: */*
    // Authorization: Basic YXNkYXM6c2Rhc2Q=

    char *line;

    char *p;

    while (line = evbuffer_readline(recv_data)) // 解析请求类型
    {
        if (p = strstr(line, "User-Agent:"))
        {
            strcpy(_connect_info->User_Agent, p + 12);
        }
        if (p = strstr(line, "Authorization: Basic"))
        {
            strcpy(_connect_info->Base64UserID, p + 21);
            // 解码base64
            Decode_base64(_connect_info->Base64UserID, _connect_info->UserName, _connect_info->Password);
        }
        else if (p = strstr(line, "Accept: */*"))
        {
            _connect_info->Base64UserID[0] = '\0';
            _connect_info->UserName[0] = '\0';
            _connect_info->Password[0] = '\0';
        }

        free(line);
    }

    return 0;
}

int tcp_connect::ntrip_server_header_decode(evbuffer *recv_data)
{
    char *line;

    char *p;

    while (line = evbuffer_readline(recv_data)) // 解析请求类型
    {
        if (p = strstr(line, "User-Agent:"))
        {
            strcpy(_connect_info->User_Agent, p + 12);
        }
        if (p = strstr(line, "Authorization: Basic"))
        {
            strcpy(_connect_info->Base64UserID, p + 21);
            // 解码base64
            Decode_base64(_connect_info->Base64UserID, _connect_info->UserName, _connect_info->Password);
        }
        else if (p = strstr(line, "Accept: */*"))
        {
            _connect_info->Base64UserID[0] = '\0';
            _connect_info->UserName[0] = '\0';
            _connect_info->Password[0] = '\0';
        }

        free(line);
    }

    return 0;
}

int tcp_connect::ntrip_relay_header_decode(int use_ntrip2)
{
    // 首先要判断这个连接是不是主动发起的relay连接，
    // 不然岂不是直接给程序整不会了

    // 判断连接类型是否是relay，以及必要的信息是否已经填充完全

    // 如果是
    _connect_info->ntrip2 = use_ntrip2;

    return 0;
}

int tcp_connect::create_connect_by_type()
{

    // 许可信息通过

    // 解除ReadCallback的绑定（但保留EventCallback，之后读取的数据都由新创建的对象来处理，但是连接断开之后同样要由EventCallback完成TCP连接表的清理工作）
    bufferevent_setcb(_bev, NULL, NULL, EventCallback, this);

    /*
    //根据过期时间，设置回调
    //计算距离过期时间还有多少秒，设置一个LicenceFailCallback

    _timeev = event_new(_svr_base,-1,EV_TIMEOUT,LicenceExpireCallback,this);
    _timeout.tv_sec =EVENT_TIMEOUT_SEC;
    _timeout.tv_usec = 0;
    event_add(_timeev, &_timeout);
    */

    switch (_connect_info->type)
    {
    case TCP_CONNECT_TYPE_CLIENT_NTRIP:
        // 如果是client
        // 创建一个对象（传入bev和许可信息） 绑定监听函数
        _ntrip_client = new ntrip_client(_svr_base, _list, _connect_info);
        _ntrip_client->start();
        break;
    case TCP_CONNECT_TYPE_SERVER_NTRIP:
        // 如果是server
        // 创建一个对象（传入fd信息和许可信息，以及连接表连接表可以有多个，用于不同框架？不同子网？）
        _ntrip_server = new ntrip_server(_svr_base, _list, _connect_info);
        _ntrip_server->start();
        break;
    case TCP_CONNECT_TYPE_GET_LIST:
        // 如果是获取源列表

        break;

    // case TCP_CONNECT_TYPE_QUEUE_GUI_CLIENT:
    //     // 如果是gui
    //     // 创建一个GUI对象，将对象加入GUI表
    //     _que_client = new que_client();
    //     _que_client->start();
    //     break;
    case TCP_CONNECT_TYPE_CLIENT_RELAY:
        // 如果是client
        // 创建一个对象（传入bev和许可信息） 绑定监听函数
        _relay_client = new relay_client();
        //_relay_client->start();

        break;
    // case TCP_CONNECT_TYPE_SERVER_RELAY:
    //     // 如果是relay
    //     // 创建一个对象（传入bev和许可信息）
    //     _relay_server = new relay_server();
    //     _relay_server->start();
    //     break;
    default:
        // 未知的/暂不支持的连接对象，关闭连接
        delay_close(10);
    }

    return 0;
}
