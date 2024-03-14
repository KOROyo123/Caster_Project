#include <iostream>

#include "event.h"
#include "event2/bufferevent.h"

#include "ntripcenter.h"
#include "connect_type.h"

#include "rtklib.h"

// #include "absl/strings/str_join.h"

#include <nlohmann/json.hpp>

#include "DB/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <hiredis.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <thread>

#include <nlohmann/json.hpp>

#include <unistd.h>
#include <arpa/inet.h>


#include "DB/relay_account_table.h"
// 

int main()
{

	// 读取用户

	// 读取第三方账户表

#ifdef NTRIP_MULT_THREAD

#ifdef WIN32
		WSADATA wsa_data;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data); // 初始化WinSock2
	if (ret != 0)
	{
		return 1;
	}
#endif

#ifdef WIN32
	evthread_use_windows_threads(); // libevent启用windows线程函数
#else
	evthread_use_pthreads(); // libenvet启用linux线程函数
#endif

#endif
	// test();
	// 新建一个ntripcenter对象
	ntripcenter *svr = new ntripcenter();
	// 设置监听端口(可以支持同时监听多个端口)
	svr->add_listern_port(4202);

	svr->server_start();
	while (1)
	{
		// 检查注册许可
		sleepms(30000);
		std::cout << "is running" << std::endl;
	}

#ifdef NTRIP_MULT_THREAD

#ifdef WIN32
	WSACleanup();
#endif

#endif
	return 0;
}


// int main333()
// {

// 	relay_account_table table;

// 	char path[] = "/Code/Koro_Caster/doc/relay_account.json";

// 	table.load_account_file(path);

// 	relay_account_entry *entry;

// 	char mount[] = "RTCM32_A1";

// 	entry = table.find_match_MapingMonut(mount);

// 	unique_account *account;
// 	while (account = table.find_idle_account_by_MapingMount(mount))
// 	{
// 		printf("%s\n", account->userName_pwd);
// 	}

// 	while (entry = table.find_idle_sys_relay_account_by_state())
// 	{
// 		printf("%s\n", entry->sys_relay_account->userName_pwd);
// 	}

// 	return 0;
// }

// int main222()
// {

// 	redisContext *c = redisConnect("127.0.0.1", 6379);
// 	if (c != NULL && c->err)
// 		printf("Error: %s\n", c->errstr); // {undefined handle error }

// 	redisReply *reply = (redisReply *)redisCommand(c, "AUTH %s", "koroyo123");
// 	if (reply->type == REDIS_REPLY_ERROR)
// 		// cout << "fail" << endl;
// 		freeReplyObject(reply);

// 	redisReply *sendredis = (redisReply *)redisCommand(c, "SET key:MOUNT_SK01 {test}");
// 	freeReplyObject(sendredis); // 返回的是一个指针，所以要释放掉才行

// 	redisReply *getredis = (redisReply *)redisCommand(c, "Get key:MOUNT_SK01");
// 	freeReplyObject(getredis);
// 	return 0;
// }


// struct work
// {
// 	event *active;

// 	event_base *base;
// 	/* data */
// };

// using json = nlohmann::json;

// void read_cb(evutil_socket_t fd, short events, void *arg)
// {

// 	char oneline[1024] = {"\0"};

// 	recv(fd, oneline, 1024, 0);

// 	printf("read_cb %s \n", oneline);

// 	std::string onestring = oneline;

// 	// json fd_decode = json::parse(onestring);
// 	json fd_decode = json::parse(oneline);

// 	evutil_socket_t decodefd = fd_decode["fd"];

// 	printf("read_cb %d \n", decodefd);
// }

// void *event_base_thread(void *arg)
// {
// #ifdef WIN32
// 	evthread_use_windows_threads(); // libevent启用windows线程函数
// #else
// 	evthread_use_pthreads(); // libenvet启用linux线程函数
// #endif

// 	evutil_socket_t *recv_fd = (evutil_socket_t *)arg;

// 	// work *svr=(work*)arg;

// 	event_base *base = event_base_new();

// 	event *active = event_new(base, *recv_fd, EV_READ | EV_PERSIST, read_cb, NULL);
// 	event_add(active, NULL);

// #ifdef WIN32
// 	WSADATA wsa_data;
// 	int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data); // 初始化WinSock2
// 	if (ret != 0)
// 	{
// 		return 1;
// 	}
// #endif

// 	evthread_make_base_notifiable(base);

// 	event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

// 	event_base_dispatch(base);

// #ifdef WIN32
// 	WSACleanup();
// #endif

// 	return 0;
// }

// void accept_cb(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg)
// {

// 	evutil_socket_t *send_fd = (evutil_socket_t *)arg;
// 	// 连接建立

// 	printf("accept_cb");

// 	const char info[] = {"ememmm"};
// 	// send(fds[1], info, sizeof(info), 0);

// 	json fd_encode;

// 	fd_encode["fd"] = fd;

// 	std::string encode_str = fd_encode.dump();

// 	char oneline[1024] = {"\0"};
// 	strcpy(oneline, encode_str.c_str());

// 	// // send(sendfd,encode_str.c_str(), ,0));
// 	send(*send_fd, oneline, 1024, 0);
// 	// 	evutil_socket_t sendfd = bufferevent_getfd(bev);
// 	// 	 send(sendfd, info, sizeof(info), 0);
// }

// int main1()
// {
// #ifdef WIN32
// 	// 创建一个socketpair即可与互相通信的两个socket，保存在fds里面
// 	evutil_socket_t fds[2];
// 	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
// 	{
// 		std::cout << "创建socketpair失败\n";
// 		return false;
// 	}
// 	// 设置成无阻赛的socket
// 	evutil_make_socket_nonblocking(fds[0]);
// 	evutil_make_socket_nonblocking(fds[1]);
// #else
// 	evutil_socket_t fds[2];
// 	// if (pipe(fds))
// 	// {
// 	// 	perror("Can't create notify pipe");
// 	// 	exit(1);
// 	// }

// 	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

// 	evutil_make_socket_nonblocking(fds[0]);
// 	evutil_make_socket_nonblocking(fds[1]);

// 	evutil_socket_t send_fd = fds[1];
// 	evutil_socket_t recv_fd = fds[0];

// #endif

// 	event_base *base = event_base_new();

// 	// 监听事件
// 	const char *_ipStr = "0.0.0.0";
// 	struct sockaddr_in _sin;
// 	unsigned int _ip;
// 	_ip = inet_addr(_ipStr); // TODO:用inet_pton()替换
// 	_sin.sin_family = AF_INET;
// 	_sin.sin_addr.s_addr = _ip;
// 	// sin.sin_addr.s_addr = ip;
// 	_sin.sin_port = htons(4202);
// 	evconnlistener *listener = evconnlistener_new_bind(base, accept_cb, &send_fd, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE, -1, (struct sockaddr *)&_sin, sizeof(_sin));

// 	pthread_t id;
// 	int ret = pthread_create(&id, NULL, event_base_thread, &recv_fd);
// 	if (ret)
// 	{
// 		return 1;
// 	}

// 	evthread_make_base_notifiable(base);

// 	event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

// 	event_base_dispatch(base);

// 	return 0;
// }
