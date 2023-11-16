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

// 需要存放在哈希表中的结构

// struct map_entry
// {
// 	HT_ENTRY(map_entry) map_node;  //为解决地址冲突的next字段
// 	unsigned key;   //键
// 	int value;    //值
// };
// //哈希函数，对键不作处理
// int hashfcn(struct map_entry *e)
// {
// 	return e->key;
// }
// //判断两个元素键是否相同，用于查找
// int equalfcn(struct map_entry *e1, struct map_entry *e2)
// {
// 	return e1->key == e2->key;
// }

// //全局的哈希map
// static HT_HEAD(key_value_map, map_entry) g_map = HT_INITIALIZER();

// //特例化对应的哈希表结构
// HT_PROTOTYPE(key_value_map, map_entry, map_node, hashfcn, equalfcn)
// HT_GENERATE(key_value_map, map_entry, map_node, hashfcn, equalfcn, 0.5, malloc, realloc, free)

// //测试函数
// void test()
// {
// 	struct map_entry elm1, elm2, elm3;
// 	elm1.key = 1;
// 	elm1.value = 19;
// 	elm1.map_node.hte_next = NULL;
// 	HT_INSERT(key_value_map, &g_map, &elm1);
// 	//第一次添加后哈希表的长度为53，因此elm2与elm1产出地址冲突
// 	elm2.key = 54;
// 	elm2.value = 48;
// 	elm2.map_node.hte_next = NULL;
// 	HT_INSERT(key_value_map, &g_map, &elm2);
// 	//
// 	elm3.key = 2;
// 	elm3.value = 14;
// 	elm3.map_node.hte_next = NULL;
// 	HT_INSERT(key_value_map, &g_map, &elm3);
// 	//打印哈希表中所有数据
// 	for (unsigned b = 0; b < g_map.hth_table_length; b++)
// 	{
// 		if (g_map.hth_table[b])
// 		{
// 			struct map_entry* elm = g_map.hth_table[b];
// 			while (elm)
// 			{
// 				printf("b:%d key:%d value:%d\n", b, elm->key, elm->value);
// 				elm = elm->map_node.hte_next;
// 			}
// 		}
// 	}
// }
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

// #include <spdlog/spdlog.h>

// int main()
// {
// 	tcp_connect_table tb;

// 	for (int i = 0; i < 20; i++)
// 	{

// 		tb.add(i*10+15);
// 	}

// 	for (int i = 0; i < 5; i++)
// 	{

// 		tb.print_list();
// 	}

// 	for (int i = 0; i < 20; i++)
// 	{

// 		tb.find_print(i*5);
// 	}

// 	return 0;
// }

#include "DB/relay_account_table.h"
int main333()
{

	relay_account_table table;

	char path[] = "/Code/Koro_Caster/doc/relay_account.json";

	table.load_account_file(path);

	relay_account_entry *entry;

	char mount[] = "RTCM32_A1";

	entry = table.find_match_MapingMonut(mount);

	unique_account *account;
	while (account = table.find_idle_account_by_MapingMount(mount))
	{
		printf("%s\n", account->userName_pwd);
	}

	while (entry = table.find_idle_sys_relay_account_by_state())
	{
		printf("%s\n", entry->sys_relay_account->userName_pwd);
	}

	return 0;
}

int main222()
{

	redisContext *c = redisConnect("127.0.0.1", 6379);
	if (c != NULL && c->err)
		printf("Error: %s\n", c->errstr); // {undefined handle error }

	redisReply *reply = (redisReply *)redisCommand(c, "AUTH %s", "koroyo123");
	if (reply->type == REDIS_REPLY_ERROR)
		// cout << "fail" << endl;
		freeReplyObject(reply);

	redisReply *sendredis = (redisReply *)redisCommand(c, "SET key:MOUNT_SK01 {test}");
	freeReplyObject(sendredis); // 返回的是一个指针，所以要释放掉才行

	redisReply *getredis = (redisReply *)redisCommand(c, "Get key:MOUNT_SK01");
	freeReplyObject(getredis);
	return 0;
}

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

struct work
{
	event *active;

	event_base *base;
	/* data */
};

using json = nlohmann::json;

void read_cb(evutil_socket_t fd, short events, void *arg)
{

	char oneline[1024] = {"\0"};

	recv(fd, oneline, 1024, 0);

	printf("read_cb %s \n", oneline);

	std::string onestring = oneline;

	// json fd_decode = json::parse(onestring);
	json fd_decode = json::parse(oneline);

	evutil_socket_t decodefd = fd_decode["fd"];

	printf("read_cb %d \n", decodefd);
}

void *event_base_thread(void *arg)
{
#ifdef WIN32
	evthread_use_windows_threads(); // libevent启用windows线程函数
#else
	evthread_use_pthreads(); // libenvet启用linux线程函数
#endif

	evutil_socket_t *recv_fd = (evutil_socket_t *)arg;

	// work *svr=(work*)arg;

	event_base *base = event_base_new();

	event *active = event_new(base, *recv_fd, EV_READ | EV_PERSIST, read_cb, NULL);
	event_add(active, NULL);

#ifdef WIN32
	WSADATA wsa_data;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data); // 初始化WinSock2
	if (ret != 0)
	{
		return 1;
	}
#endif

	evthread_make_base_notifiable(base);

	event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

	event_base_dispatch(base);

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}

void accept_cb(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void *arg)
{

	evutil_socket_t *send_fd = (evutil_socket_t *)arg;
	// 连接建立

	printf("accept_cb");

	const char info[] = {"ememmm"};
	// send(fds[1], info, sizeof(info), 0);

	json fd_encode;

	fd_encode["fd"] = fd;

	std::string encode_str = fd_encode.dump();

	char oneline[1024] = {"\0"};
	strcpy(oneline, encode_str.c_str());

	// // send(sendfd,encode_str.c_str(), ,0));
	send(*send_fd, oneline, 1024, 0);
	// 	evutil_socket_t sendfd = bufferevent_getfd(bev);
	// 	 send(sendfd, info, sizeof(info), 0);
}

int main1()
{
#ifdef WIN32
	// 创建一个socketpair即可与互相通信的两个socket，保存在fds里面
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		std::cout << "创建socketpair失败\n";
		return false;
	}
	// 设置成无阻赛的socket
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	evutil_socket_t fds[2];
	// if (pipe(fds))
	// {
	// 	perror("Can't create notify pipe");
	// 	exit(1);
	// }

	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);

	evutil_socket_t send_fd = fds[1];
	evutil_socket_t recv_fd = fds[0];

#endif

	event_base *base = event_base_new();

	// 监听事件
	const char *_ipStr = "0.0.0.0";
	struct sockaddr_in _sin;
	unsigned int _ip;
	_ip = inet_addr(_ipStr); // TODO:用inet_pton()替换
	_sin.sin_family = AF_INET;
	_sin.sin_addr.s_addr = _ip;
	// sin.sin_addr.s_addr = ip;
	_sin.sin_port = htons(4202);
	evconnlistener *listener = evconnlistener_new_bind(base, accept_cb, &send_fd, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE, -1, (struct sockaddr *)&_sin, sizeof(_sin));

	pthread_t id;
	int ret = pthread_create(&id, NULL, event_base_thread, &recv_fd);
	if (ret)
	{
		return 1;
	}

	evthread_make_base_notifiable(base);

	event_base_dump_events(base, stdout); // 向控制台输出当前已经绑定在base上的事件

	event_base_dispatch(base);

	return 0;
}
