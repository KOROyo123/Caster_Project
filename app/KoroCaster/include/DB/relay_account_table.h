#pragma once
#include <iostream>
#include <mutex> //unique_lock
#include <shared_mutex>
#include <set>
#include <string.h>
#include "DB/hash_table.h"

// 声明哈希表结构
//  定义哈希表的类型名和存储的元素
HT_HEAD(relay_account_ht, relay_account_entry);

struct unique_account
{
    char userName_pwd[64];
    char Base64_userID[64];

    int state;
};

struct relay_account
{
    unique_account *account;

    relay_account *next;
};

struct relay_account_entry
{
    HT_ENTRY(relay_account_entry)
    relay_account_node;

    char MapingMount[64];

    char addr[64];
    int port;
    char path[64];

    // 用户使用
    relay_account *account_list; // 账号是唯一的
    relay_account *account_last; // 账号列表的尾部
    int account_num;

    // 系统使用
    unique_account *sys_relay_account;
    // 是否发送虚拟GGA
    int Intv_of_Send_Virtual_GGA; // 时间间隔 大于0 才会开启 最小间隔为5
    double Virtual_GGA_llh[3];
};

class relay_account_table
{
private:
    std::shared_mutex _lock;

    relay_account_ht *_table;

    std::set<std::string> Maping_Mount_list; // 用户使用

    // 统计信息
    int _sum_of_user_account = 0;
    int _num_of_using_user_account = 0;

    int _sum_of_sys_relay_account = 0;
    int _sum_of_using_user_account = 0;

    /* data */
public:
    relay_account_table();
    ~relay_account_table();

    int load_account_file(char *conf_path);

    // 系统转发
    int add_new_sys_relay_mount(const char *MapingMount, const char *addr, int port, const char *mount, const char *user_pwd);
    relay_account_entry *find_idle_sys_relay_account_by_state();

    // 用户调用
    int add_new_maping_mount(const char *MapingMount, const char *addr, int port, const char *mount);
    unique_account *create_unique_account(const char *user_pwd);
    int add_unique_account(const char *MapingMount, unique_account *account);
    int search_Maping_Mount_list(char *MapingMount); // 查找是否支持该挂载点

    relay_account_entry *find_match_MapingMonut(char *MapingMount);
    unique_account *find_idle_account_by_MapingMount(char *MapingMount); // 同时会标记账号为已使用  无账号则是无可用账号

    // 通用
    int give_back_account(unique_account *using_account);
    // 更新账号为可用
};
