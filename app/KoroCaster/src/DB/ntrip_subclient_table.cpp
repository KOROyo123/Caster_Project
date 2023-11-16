
#include "DB/ntrip_subclient_table.h"

// 哈希函数，对键不作处理
static int ntrip_subclient_ht_cal_key(struct ntrip_subclient_entry *e)
{
    return HT_string_hash_(e->submount);
}
// 判断两个元素键是否相同，用于查找
static int ntrip_subclient_ht_equel(struct ntrip_subclient_entry *e1, struct ntrip_subclient_entry *e2)
{

    if (strcmp(e1->submount, e2->submount)) // 相等则等于0
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
HT_PROTOTYPE(ntrip_subclient_ht, ntrip_subclient_entry, ntrip_subclient_node, ntrip_subclient_ht_cal_key, ntrip_subclient_ht_equel)
HT_GENERATE(ntrip_subclient_ht, ntrip_subclient_entry, ntrip_subclient_node, ntrip_subclient_ht_cal_key, ntrip_subclient_ht_equel, 0.5, malloc, realloc, free)

ntrip_subclient_table::ntrip_subclient_table()
{
    _table = (ntrip_subclient_ht *)malloc(sizeof(ntrip_subclient_ht));
    HT_INIT(ntrip_subclient_ht, _table);
}
ntrip_subclient_table::~ntrip_subclient_table()
{
}

// 读锁
int ntrip_subclient_table::share_lock() { return 0; }
int ntrip_subclient_table::share_unlock() { return 0; }

// 一般都是单个操作写锁，嵌入到内部
int ntrip_subclient_table::unique_lock() { return 0; }
int ntrip_subclient_table::unique_unlock() { return 0; }

// client操作
int ntrip_subclient_table::add_sub_client(char *submount, char *userName, bufferevent *bev)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_subclient_entry *entry = (ntrip_subclient_entry *)malloc(sizeof(ntrip_subclient_entry));

    // 根据fd解析连接？

    strcpy(entry->submount, submount);
    strcpy(entry->userName, userName);
    entry->fd = bufferevent_getfd(bev);
    entry->bev = bev;

    HT_INSERT(ntrip_subclient_ht, _table, entry);

    return 0;
}
int ntrip_subclient_table::del_sub_client(char *submount, char *userName)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    ntrip_subclient_entry entry;

    strcpy(entry.submount, submount);
    strcpy(entry.userName, userName);

    ntrip_subclient_entry *find_entry = HT_REMOVE(ntrip_subclient_ht, _table, &entry);

    if (find_entry)
    {
        free(find_entry);
    }

    return 0;
}

// server操作
ntrip_subclient_entry *ntrip_subclient_table::find_client_by_submount(char *submount)
{
// 共享锁定
#ifdef ENABLE_TABLE_MUTEX
    std::shared_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock_shared();
#endif

    ntrip_subclient_entry entry;

    strcpy(entry.submount, submount);
    entry.userName[0] = '\0';

    ntrip_subclient_entry *find;

    find = HT_FIND(ntrip_subclient_ht, _table, &entry);

    return find;
} // 返回第一个哈希桶

ntrip_subclient_entry *ntrip_subclient_table::find_next_subclient(ntrip_subclient_entry *entry)
{
// 共享锁定
#ifdef ENABLE_TABLE_MUTEX
    std::shared_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock_shared();
#endif

    ntrip_subclient_entry *match_entry;

    match_entry = entry->ntrip_subclient_node.hte_next;

    while (match_entry)
    {
        if (strcmp(entry->submount, match_entry->submount))
        {
            // 不匹配，查找下一个
            match_entry = match_entry->ntrip_subclient_node.hte_next;
        }
        else
        {
            // 匹配到，返回
            return match_entry;
        }
    }
    // 遍历结束，返回
    return NULL;
}