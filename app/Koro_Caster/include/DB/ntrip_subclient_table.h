#pragma once
#include <iostream>
#include <mutex>    //unique_lock
#include<shared_mutex>
#include "DB/hash_table.h"
#include "event2/util.h"
#include "event2/bufferevent.h"

// 声明哈希表结构
//  定义哈希表的类型名和存储的元素
HT_HEAD(ntrip_subclient_ht, ntrip_subclient_entry);

struct ntrip_subclient_entry
{
    HT_ENTRY(ntrip_subclient_entry)
    ntrip_subclient_node;

    char submount[64]; // 挂载点 用来计算key
    char userName[64];

    evutil_socket_t fd;
    bufferevent *bev;
};

class ntrip_subclient_table
{
private:
    std::shared_mutex _lock;
    ntrip_subclient_ht *_table;

    /* data */
public:
    ntrip_subclient_table();
    ~ntrip_subclient_table();

    //读锁
    int share_lock();
    int share_unlock();

    //一般都是单个操作写锁，嵌入到内部
    int unique_lock(); 
    int unique_unlock(); 


    //client操作
    int add_sub_client(char *submount,char *userName,bufferevent *bev);
    int del_sub_client(char *submount,char *userName);

    //server操作
    ntrip_subclient_entry* find_client_by_submount(char *submount);//返回第一个哈希桶
    ntrip_subclient_entry* find_next_subclient(ntrip_subclient_entry* entry);//找到当前entry的下一个，挂载点相同的node，
    //（主要是为了避免不同的挂载点计算的哈希值相同而被分配到同一个桶中）

    //ntrip_subclient_entry* find_client_by_userName(char *userName);//遍历，效率较低
    //ntrip_subclient_entry* find_client_by_sublocalmount_and_userName(char *localmount,char *userName);//从挂载点的订阅者中找userName

};
