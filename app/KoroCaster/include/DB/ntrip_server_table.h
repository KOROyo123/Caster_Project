#pragma once
#include <iostream>
#include <mutex>    //unique_lock
#include <shared_mutex>
#include "DB/hash_table.h"
#include "event2/util.h"
#include "event2/bufferevent.h"

//声明哈希表结构
// 定义哈希表的类型名和存储的元素
HT_HEAD(ntrip_server_ht, ntrip_server_entry);


struct ntrip_server_entry
{
	HT_ENTRY(ntrip_server_entry) ntrip_server_node;
	
    char localmount[64]; //挂载点 用来计算key 

    char userName[64];  

	evutil_socket_t fd;
    bufferevent *bev;
};



class ntrip_server_table
{
private:
    std::shared_mutex _lock;
    ntrip_server_ht *_table;

    /* data */
public:
    ntrip_server_table();
    ~ntrip_server_table();

    //读锁
    int share_lock();
    int share_unlock();

    //一般都是单个操作写锁，嵌入到内部
    int unique_lock(); 
    int unique_unlock(); 


    //server操作
    int add(char *localmount,char *userName,bufferevent *bev);
    int del(char *localmount,char *userName);


    //client/server操作
    ntrip_server_entry * is_exist(char *localmount);//查找挂载点是否在线

};
