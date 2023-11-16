#include "DB/tcp_connect_table.h"

// 哈希函数，对键不作处理
static int tcp_connect_ht_cal_key(struct tcp_connect_entry *e)
{
    return HT_improve_hash_(e->fd);
}
// 判断两个元素键是否相同，用于查找
static int tcp_connect_ht_equel(struct tcp_connect_entry *e1, struct tcp_connect_entry *e2)
{
    // 通过挂载点名查找，挂载点生成key，利用key检索？
    // 检查挂载点名是否一致
    return e1->fd == e2->fd;
}

// 特例化对应的哈希表结构
HT_PROTOTYPE(tcp_connect_ht, tcp_connect_entry, tcp_connect_node, tcp_connect_ht_cal_key, tcp_connect_ht_equel)
HT_GENERATE(tcp_connect_ht, tcp_connect_entry, tcp_connect_node, tcp_connect_ht_cal_key, tcp_connect_ht_equel, 0.5, malloc, realloc, free)

tcp_connect_table::tcp_connect_table(/* args */)
{
    // tcp_connect_table_init(&table);
    _table = (tcp_connect_ht *)malloc(sizeof(tcp_connect_ht));
    HT_INIT(tcp_connect_ht, _table);
}

tcp_connect_table::~tcp_connect_table()
{
}

// 读锁
int tcp_connect_table::share_lock() { return 0; }
int tcp_connect_table::share_unlock() { return 0; }

// 一般都是单个操作写锁，嵌入到内部
int tcp_connect_table::unique_lock() { return 0; }
int tcp_connect_table::unique_unlock() { return 0; }

int tcp_connect_table::add(evutil_socket_t fd)
{
    // 唯一锁定
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();

    tcp_connect_entry *entry = (tcp_connect_entry *)malloc(sizeof(tcp_connect_entry));

    // 根据fd解析连接？
    entry->fd = fd;

    HT_INSERT(tcp_connect_ht, _table, entry);
    return 0;
}

int tcp_connect_table::del(evutil_socket_t fd)
{
    // 唯一锁定
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();

    tcp_connect_entry entry;
    entry.fd = fd;

    tcp_connect_entry *find_entry = HT_REMOVE(tcp_connect_ht, _table, &entry);

    if (find_entry)
    {
        free(find_entry);
    }

    return 0;
}

// int tcp_connect_table::outputlist()
// {

//     return 0;
// }

// int tcp_connect_table::print_list()
// {
//     tcp_connect_entry **entry;

//     entry = HT_START(tcp_connect_ht, _table);

//     do
//     {
//         printf("%4d", (*entry)->fd);

//     } while (entry = HT_NEXT(tcp_connect_ht, _table, entry));

//     printf("\n");

//     return 0;
// }
// int tcp_connect_table::find_print(evutil_socket_t fd)
// {

//     tcp_connect_entry entry;
//     entry.fd = fd;

//     tcp_connect_entry *find;

//     find = HT_FIND(tcp_connect_ht, _table, &entry);

//     if(find==NULL)
//     {
//          printf("\n");
//         return 0;
//     }

//     tcp_connect_entry **find2=&find;
//     do
//     {
//         if((*find2)->fd%100!=fd)
//         {
//             break;
//         }

//         printf("%4d", (*find2)->fd);

//     } while (find2 = HT_NEXT(tcp_connect_ht, _table, find2));

//     printf("\n");

//     return 0;
// }