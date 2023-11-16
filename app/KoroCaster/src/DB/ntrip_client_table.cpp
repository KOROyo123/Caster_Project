
#include "DB/ntrip_client_table.h"

// 哈希函数，对键不作处理
static int ntrip_client_ht_cal_key(struct ntrip_client_entry *e)
{
    return HT_string_hash_(e->userName);
}
// 判断两个元素键是否相同，用于查找
static int ntrip_client_ht_equel(struct ntrip_client_entry *e1, struct ntrip_client_entry *e2)
{
    if (strcmp(e1->userName, e2->userName)) // 相等则等于0
    {
        return 0; // 挂载点名不匹配
    }
    else if (e2->localmount[0] == '\0')
    {
        return 1; // 挂载点名匹配，不匹配userName，直接返回
    }
    else if (strcmp(e1->localmount, e2->localmount))
    {
        return 0; // 用户名不匹配
    }

    return 1;
}

// 特例化对应的哈希表结构
HT_PROTOTYPE(ntrip_client_ht, ntrip_client_entry, ntrip_client_node, ntrip_client_ht_cal_key, ntrip_client_ht_equel)
HT_GENERATE(ntrip_client_ht, ntrip_client_entry, ntrip_client_node, ntrip_client_ht_cal_key, ntrip_client_ht_equel, 0.5, malloc, realloc, free)

ntrip_client_table::ntrip_client_table()
{
    _table = (ntrip_client_ht *)malloc(sizeof(ntrip_client_ht));
    HT_INIT(ntrip_client_ht, _table);
}
ntrip_client_table::~ntrip_client_table()
{
}

// 读锁
int ntrip_client_table::share_lock()
{
    return 0;
}
int ntrip_client_table::share_unlock()
{
    return 0;
}

// 一般都是单个操作写锁，嵌入到内部
int ntrip_client_table::unique_lock()
{
    return 0;
}
int ntrip_client_table::unique_unlock()
{
    return 0;
}

// server操作
int ntrip_client_table::add(char *localmount, char *userName, bufferevent *bev)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_client_entry *entry = (ntrip_client_entry *)malloc(sizeof(ntrip_client_entry));

    // 根据fd解析连接？
    strcpy(entry->localmount, localmount);
    strcpy(entry->userName, userName);
    entry->fd = bufferevent_getfd(bev);
    entry->bev = bev;

    HT_INSERT(ntrip_client_ht, _table, entry);

    return 0;
}
int ntrip_client_table::del(char *localmount, char *userName)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_client_entry entry;

    strcpy(entry.localmount, localmount);
    strcpy(entry.userName, userName);

    ntrip_client_entry *find_entry = HT_REMOVE(ntrip_client_ht, _table, &entry);

    if (find_entry)
    {
        free(find_entry);
    }

    return 0;
}

// client操作
int ntrip_client_table::is_exist(char *userName)
{
    // 共享锁定
    std::shared_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock_shared();



    return 0;
}