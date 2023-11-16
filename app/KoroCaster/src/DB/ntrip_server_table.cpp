
#include "DB/ntrip_server_table.h"

// 哈希函数，对键不作处理
static int ntrip_server_ht_cal_key(struct ntrip_server_entry *e)
{
    return HT_string_hash_(e->localmount);
}
// 判断两个元素键是否相同，用于查找
static int ntrip_server_ht_equel(struct ntrip_server_entry *e1, struct ntrip_server_entry *e2)
{
    if (strcmp(e1->localmount, e2->localmount)) // 相等则等于0
    {
        return 0; // 挂载点名不匹配
    }
    else if (e2->userName[0] == '\0')
    {
        return 1; // 挂载点名匹配，不匹配userName，直接返回
    }
    else if (strcmp(e1->userName, e2->userName))
    {
        return 0; // 用户名不匹配
    }

    return 1;
}

// 特例化对应的哈希表结构
HT_PROTOTYPE(ntrip_server_ht, ntrip_server_entry, ntrip_server_node, ntrip_server_ht_cal_key, ntrip_server_ht_equel)
HT_GENERATE(ntrip_server_ht, ntrip_server_entry, ntrip_server_node, ntrip_server_ht_cal_key, ntrip_server_ht_equel, 0.5, malloc, realloc, free)

ntrip_server_table::ntrip_server_table()
{
    _table = (ntrip_server_ht *)malloc(sizeof(ntrip_server_ht));
    HT_INIT(ntrip_server_ht, _table);
}
ntrip_server_table::~ntrip_server_table()
{
}

// 读锁
int ntrip_server_table::share_lock() { return 0; }
int ntrip_server_table::share_unlock() { return 0; }

// 一般都是单个操作写锁，嵌入到内部
int ntrip_server_table::unique_lock() { return 0; }
int ntrip_server_table::unique_unlock() { return 0; }

// server操作
int ntrip_server_table::add(char *localmount, char *userName, bufferevent *bev)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_server_entry *entry = (ntrip_server_entry *)malloc(sizeof(ntrip_server_entry));

    // 根据fd解析连接？
    strcpy(entry->localmount, localmount);
    strcpy(entry->userName, userName);
    entry->fd = bufferevent_getfd(bev);
    entry->bev = bev;

    HT_INSERT(ntrip_server_ht, _table, entry);

    return 0;
}
int ntrip_server_table::del(char *localmount, char *userName)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_server_entry entry;

    strcpy(entry.localmount, localmount);
    strcpy(entry.userName, userName);

    ntrip_server_entry *find_entry = HT_REMOVE(ntrip_server_ht, _table, &entry);

    if (find_entry)
    {
        free(find_entry);
    }

    return 0;
}

// client操作
ntrip_server_entry *ntrip_server_table::is_exist(char *localmount)
{
// 共享锁定
#ifdef ENABLE_TABLE_MUTEX
    std::shared_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock_shared();
#endif

    ntrip_server_entry entry;

    strcpy(entry.localmount, localmount);
    entry.userName[0] = '\0';

    ntrip_server_entry *find;

    find = HT_FIND(ntrip_server_ht, _table, &entry);

    return find;
}