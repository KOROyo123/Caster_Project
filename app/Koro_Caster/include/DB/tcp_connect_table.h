#pragma once
#include <iostream>
#include <mutex>    //unique_lock
#include <shared_mutex>
#include "DB/hash_table.h"
#include "event2/util.h"
#include "event.h"

// 声明哈希表结构
//  定义哈希表的类型名和存储的元素
HT_HEAD(tcp_connect_ht, tcp_connect_entry);

struct tcp_connect_entry
{
	HT_ENTRY(tcp_connect_entry)
	tcp_connect_node;

	// 连接的fd 用来计算key
	evutil_socket_t fd;

	event *process_cmd;  //可以通过tcp连接表，检索fd，激活该函数，来处理指令
	char* cmd;

};


class tcp_connect_table
{
private:
    std::shared_mutex _lock;
	tcp_connect_ht *_table;

	/* data */
public:
	tcp_connect_table(/* args */);
	~tcp_connect_table();


    //读锁
    int share_lock();
    int share_unlock();

    //一般都是单个操作写锁，嵌入到内部
    int unique_lock(); 
    int unique_unlock(); 

	int add(evutil_socket_t fd);
	int del(evutil_socket_t fd);

	// int outputlist();

	// int print_list();

	// int find_print(evutil_socket_t fd);

	//tcp_connect_entry *find(evutil_socket_t fd);
};
