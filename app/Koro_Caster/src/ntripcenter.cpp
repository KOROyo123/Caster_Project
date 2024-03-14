#include "ntripcenter.h"

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <thread>
#include <iostream>

#include "connect_type.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

ntripcenter::ntripcenter(ntripcenter_list *list)
{

	reset();
	//_port = 0;
	init_event_base();
	init_socket_pair();
	init_pair_event();
	// 创建事件pair监听事件

	_list = list;
}

ntripcenter::ntripcenter()
{
	reset();
	//_port = 0;
	init_event_base();
	init_socket_pair();
	init_pair_event();
	// 创建事件pair监听事件

	_list = new ntripcenter_list();
}

ntripcenter::~ntripcenter()
{
}

int ntripcenter::reset()
{
	_weight = 0;
	_send_fd = 0;
	_recv_fd = 0;
	_svr_base = nullptr;
	return 0;
}

int ntripcenter::load_conf(char *path)
{
	return 0;
}

int ntripcenter::add_listern_port(int port)
{
	_port.push_back(port);

	return 0;
}

int ntripcenter::server_start()
{

#ifdef NTRIP_MULT_THREAD
	start_mult_thread();
#else
	start_single_thread();
#endif

	return 0;
}

int ntripcenter::start_single_thread()
{
	// 依次绑定listern的端口
	for (int i = 0; i < _port.size(); i++)
	{
		lister_to(_port.at(i));
	}

	_ntripcenter.push_back(this);

	// 创建一个新的线程，在新的线程中启动base_dispatch
	start();

	return 0;
}

int ntripcenter::start_mult_thread()
{

	// 依次绑定listern的端口
	for (int i = 0; i < _port.size(); i++)
	{
		lister_to(_port.at(i));
	}
	// 创建一个子线程
	create_ntripcenter();

	// 创建一个子线程
	create_ntripcenter();

	start();

	return 0;
}

int ntripcenter::start()
{

	// lister_to(_svr_base,port);

	start_server_thread();

	return 0;
}

int ntripcenter::stop()
{
	return 0;
}

int ntripcenter::restart()
{
	return 0;
}

void ntripcenter::AcceptConnectionCallback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg)
{
	ntripcenter *svr = (ntripcenter *)arg;

	printf("Accept Connect %d \n", fd);

	ntripcenter *send_svr;
	evutil_socket_t send_fd;
	// 连接建立

	// 按照一定的规则，找出要分配的对象
	int min_weight = WEIGHT_LIMIT, select_svr = -1;
	for (int i = 0; i < svr->_ntripcenter.size(); i++)
	{
		if (min_weight > svr->_ntripcenter.at(i)->_weight)
		{
			min_weight = svr->_ntripcenter.at(i)->_weight;
			select_svr = i;
		}
	}
	// 如果没有合适的对象，那么新建一个
	if (select_svr == -1)
	{
		svr->create_ntripcenter();
		send_svr = svr->_ntripcenter.back();
	}
	else
	{
		send_svr = svr->_ntripcenter.at(select_svr);
	}

	json fd_encode;

	fd_encode["type"] = REQUEST_NEW_CONNECT_ALLOC;
	fd_encode["fd"] = fd;

	std::string encode_str = fd_encode.dump();

	encode_str += "\r\n";

	// // 同时把任务放到队列中去
	// // send_svr->_queue_lock.lock(); 单线程操作 不需要锁
	// send_svr->_queue.push_back(encode_str);
	// // send_svr->_queue_lock.unlock();

	// 激活函数
	// send(send_fd, encode_str.c_str(), encode_str.size(), 0);
	bufferevent_write(send_svr->_send_bev, encode_str.c_str(), encode_str.size());
}

void ntripcenter::AcceptErrorCallback(evconnlistener *listener, void *arg)
{
	ntripcenter *svr = (ntripcenter *)arg;
}

void ntripcenter::Request_process_cb(bufferevent *bev, void *arg)
{
	ntripcenter *svr = (ntripcenter *)arg;

	evbuffer *evbuf = bufferevent_get_input(bev);
	// 读取数据 一次只处理一个请求就够了
	char *line;

	// bufferevent_read(bev,line,1024);
	while (line = evbuffer_readline(evbuf))
	{
		// 转string
		std::string oneque = line;

		free(line);
		// 反序列化
		json fd_decode = json::parse(oneque);

		evutil_socket_t decodefd = fd_decode["fd"];

		// 新建一个tcp对象，传入参数
		tcp_connect *one_connect = new tcp_connect(svr->_svr_base, svr->_list, svr->_send_bev, TCP_CONNECT_TYPE_COMMON_NTRIP, decodefd);

		// 绑定回调函数，准备验证
		one_connect->start();
	}
}

int ntripcenter::Process_Request(json req)
{
	//主要功能 
	//1、处理TCP连接请求（主动，和被动）  创建一个tcp connect连接对象

	//2、处理指令

	//3、处理通知，根据配置和以及通知做出响应（如基站通知下线了，则要响应踢出挂载在该基站下的移动站）（）


	switch ((int)req["type"])
	{
	case REQUEST_NEW_CONNECT_ALLOC:
		break;

	case REQUEST_THIRD_PART_SERVER_LINK:
		// connect_to()
		break;

	case REQUEST_THIRD_PART_SERVER_ALLOC:

		break;
	default:
		break;
	}

	return 0;
}

/* 创建一个主动连接，并绑定到指定的base上(并不控制base启动)
 * args   : event_base 		*base    		I
 *          relay_info_t 	relay_info    	I
 * return : status (0:ok 1:error)
 * note	  :
 */
int ntripcenter::connect_to(char *addr, int port)
{
	struct bufferevent *bev;

	// 将IP地址传入sin
	struct sockaddr_in sin = {0};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(addr);
	sin.sin_port = htons(port);

	// 创建一个绑定在base上的buffevent，并建立socket连接
	bev = bufferevent_socket_new(_svr_base, -1, BEV_OPT_THREADSAFE); //-1表示自动创建fd
	if(bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)))
	{
		return 1;  //建立连接失败
	}
	evutil_socket_t fd = bufferevent_getfd(bev);

	// 注册事件回调
	bufferevent_setcb(bev, NULL, NULL, AccessConnectCallback, this);
	bufferevent_enable(bev, EV_READ); // TODO:参数意义

	return 0;
}

void ntripcenter::AccessConnectCallback(struct bufferevent *bev, short what, void *arg)
{
	// 新建一个对象

	// 传入参数

	// 释放参数
}
int ntripcenter::lister_to(int port)
{

	_ip = inet_addr(_ipStr); // TODO:用inet_pton()替换

	_sin.sin_family = AF_INET;
	_sin.sin_addr.s_addr = _ip;
	// sin.sin_addr.s_addr = ip;
	_sin.sin_port = htons(port);

	evconnlistener *listener = evconnlistener_new_bind(_svr_base, AcceptConnectionCallback, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE, -1, (struct sockaddr *)&_sin, sizeof(_sin));

	evconnlistener_set_error_cb(listener, AcceptErrorCallback);

	// 添加到监听队列
	_listener.push_back(listener);

	return 0;
}

/* 监听指定端口，并绑定到指定的base上(并不启动base)
 * args   : event_base *base    I
 *          int 		port    I
 * return : status (1:ok 0:error)
 * note	  :
 */
// int ntripcenter::lister_to(event_base *base, int port)
// {

// 	return 0;
// }

int ntripcenter::start_server_thread()
{

	pthread_t id;
	int ret = pthread_create(&id, NULL, event_base_thread, _svr_base);
	if (ret)
	{
		return 1;
	}
	return 0;
}

int ntripcenter::create_ntripcenter()
{
	_ntripcenter.push_back(new ntripcenter());

	_ntripcenter.back()->start();

	return 0;
}

/*	启动一个线程，运行event_base_dispatch
 * args   : event_base *base    I
 * return :
 * note	  :	never return
 */
void *ntripcenter::event_base_thread(void *arg)
{
	event_base *base = (event_base *)arg;

	evthread_make_base_notifiable(base);

	event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

	event_base_dispatch(base);

	return 0;
}

int ntripcenter::init_event_base()
{
	_svr_base = event_base_new();

	return 0;
}

int ntripcenter::init_socket_pair()
{
	evutil_socket_t fds[2];

	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);

	_send_fd = fds[1];
	_recv_fd = fds[0];
	return 0;
}

int ntripcenter::init_pair_event()
{
// BEV_OPT_CLOSE_ON_FREE：释放bufferevent自动关闭底层接口
// BEV_OPT_THREADSAFE：使bufferevent能够在多线程下是安全的
//  注册事件回调
#ifdef NTRIP_MULT_THREAD
	_recv_bev = bufferevent_socket_new(_svr_base, _recv_fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE); //-1表示自动创建fd
	_send_bev = bufferevent_socket_new(_svr_base, _send_fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE); //-1表示自动创建fd
#else
	_recv_bev = bufferevent_socket_new(_svr_base, _recv_fd, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
	_send_bev = bufferevent_socket_new(_svr_base, _send_fd, BEV_OPT_CLOSE_ON_FREE); //-1表示自动创建fd
#endif
	// bufferevent *bev_pair[2];
	// bufferevent_pair_new(_svr_base,BEV_OPT_THREADSAFE,bev_pair);
	// _send_bev=bev_pair[0];
	// _recv_bev=bev_pair[1];

	bufferevent_setcb(_recv_bev, Request_process_cb, NULL, NULL, this);
	bufferevent_enable(_recv_bev, EV_READ); // TODO:参数意义
	bufferevent_enable(_recv_bev, 0);		// TODO:参数意义

	return 0;
}
