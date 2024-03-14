#pragma once

#include "event.h"

#include <iostream>
#include <vector>
#include <functional>


#include "connect_type.h"

#include "DB/tcp_connect_table.h"

#include <nlohmann/json.hpp>
#include <hiredis.h>

using json = nlohmann::json;

// 单个线程的连接负载，多线程模式下，超过负载后会新建一个线程
#define WEIGHT_LIMIT 5000
// （线程回收制度，后续可改为，并不是优先分配给数量最低的线程，而是优先分配给最接近负载限制的线程，只有所有线程都满了的情况下，才会开新的线程
// 这样  负载低的线程会逐渐断开，降低负载，当负载为0的时候，就自动释放掉？



class ntripcenter
{
public:
	ntripcenter(ntripcenter_list *list);
	ntripcenter();
	~ntripcenter();

public:
	int load_conf(char *path);
	int add_listern_port(int port);


	int server_start();

	int start_single_thread();
	int start_mult_thread();

	int start();
	int stop();
	int restart();

private:
	// 主动模式
	int connect_to(char *addr,int port);
	// 被动模式
	int lister_to(int port);

	// libevent回调用函数（单线程回调函数）
	static void AcceptConnectionCallback(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg);
	static void AcceptErrorCallback(struct evconnlistener *listener, void *arg);

	// 主动连接相关回调
	static void AccessConnectCallback(struct bufferevent *bev, short what, void *arg);

private:
	static void Request_process_cb(bufferevent *bev, void *arg);
	// 请求处理函数
	// 传入json，进行处理
	int Process_Request(json req);

private:
	int create_ntripcenter();

	int init_event_base();
	int init_socket_pair();
	int init_pair_event();

	int reset();

	int start_server_thread();
	static void *event_base_thread(void *arg);

private:
	// 多端口监听
	std::vector<int> _port;
	std::vector<evconnlistener *> _listener;

	// 多线程服务（父线程管理所有子线程，以及向子线程分发任务用的成员）
	std::vector<ntripcenter *> _ntripcenter;

	// 当前线程的负载（指事件数量）
	int _weight; // 当前负载状况的一个评价值，越高则连接数越多

	// 内部通讯的管道
	evutil_socket_t _send_fd;
	evutil_socket_t _recv_fd;
	bufferevent *_recv_bev;
	bufferevent *_send_bev;

	// 绑定的event_base
	event_base *_svr_base;

	// 记录表
	ntripcenter_list *_list;

	// 启动的地址（仅有一个线程需要这个）
	struct sockaddr_in _sin;
	unsigned int _ip;
	const char *_ipStr = "0.0.0.0";
};
