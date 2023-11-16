#pragma once
#include <iostream>
#include <mutex>    //unique_lock
#include <shared_mutex>
#include "DB/hash_table.h"
#include "event2/util.h"
#include "event2/bufferevent.h"

//声明哈希表结构
// 定义哈希表的类型名和存储的元素
HT_HEAD(ntrip_client_ht, ntrip_client_entry);

struct ntrip_client_entry
{
	HT_ENTRY(ntrip_client_entry) ntrip_client_node;

    char userName[64];  //用户名 用来计算key 
    
    char localmount[64];

	evutil_socket_t fd;
    bufferevent *bev;

    int multflag;//标识码 同一账户重复使用标记
};



class ntrip_client_table
{
private:
    std::shared_mutex _lock;

    ntrip_client_ht *_table;

    /* data */
public:
    ntrip_client_table();
    ~ntrip_client_table();

    //读锁
    int share_lock();
    int share_unlock();

    //一般都是单个操作写锁，嵌入到内部
    int unique_lock(); 
    int unique_unlock(); 


    //server操作
    int add(char *localmount,char *userName,bufferevent *bev);
    int del(char *localmount,char *userName);


    //client操作
    int is_exist(char *userName);//查找用户是否已经使用

};
