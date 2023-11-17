#include "DB/relay_account_table.h"
#include <fstream>
#include <nlohmann/json.hpp>

#include "knt/base64.h"

using json = nlohmann::json;

// 哈希函数，对键不作处理
static int relay_account_ht_cal_key(struct relay_account_entry *e)
{
    return HT_string_hash_(e->MapingMount);
}
// 判断两个元素键是否相同，用于查找
static int relay_account_ht_equel(struct relay_account_entry *e1, struct relay_account_entry *e2)
{

    if (strcmp(e1->MapingMount, e2->MapingMount)) // 相等则等于0
    {
        return 0; // 挂载点名不匹配
    }
    return 1;
}

// 特例化对应的哈希表结构
HT_PROTOTYPE(relay_account_ht, relay_account_entry, relay_account_node, relay_account_ht_cal_key, relay_account_ht_equel)
HT_GENERATE(relay_account_ht, relay_account_entry, relay_account_node, relay_account_ht_cal_key, relay_account_ht_equel, 0.5, malloc, realloc, free)

relay_account_table::relay_account_table()
{

    _table = (relay_account_ht *)malloc(sizeof(relay_account_ht));
    HT_INIT(relay_account_ht, _table);
}

relay_account_table::~relay_account_table()
{
}

int relay_account_table::load_account_file(char *conf_path)
{
    FILE *listfp = fopen(conf_path, "r");
    if (listfp == NULL)
    {
        // 不存在文件
        return false;
    }
    std::ifstream f(conf_path);
    json data = json::parse(f);

    //  读取用户转发配置
    for (int i = 0; i < data["user_config"].size(); i++)
    {
        json one_conf = data["user_config"].at(i);

        for (int j = 0; j < one_conf["username_pwd"].size(); j++)
        {
            std::string user_pwd;
            user_pwd = one_conf["username_pwd"].at(j);

            unique_account *one_account = create_unique_account(user_pwd.c_str());

            for (int k = 0; k < one_conf["user_config"].size(); k++)
            {
                json maping = one_conf["Maping"].at(j);
                std::string maping_Mount, addr, mount;
                int port;

                maping_Mount = maping["Maping_Mount"];
                addr = maping["addr"];
                port = maping["port"];
                mount = maping["Mount"];

                add_new_maping_mount(maping_Mount.c_str(), addr.c_str(), port, mount.c_str());
            }
        }
    }

    // 读取系统转发配置
    for (int i = 0; i < data["sys_config"].size(); i++)
    {
        json item = data["sys_config"].at(i);

        std::string maping_Mount, addr, mount, user_pwd;
        int port;

        maping_Mount = item["Local_Mount"];
        addr = item["addr"];
        port = item["port"];
        mount = item["Mount"];
        user_pwd = item["username_pwd"];

        add_new_sys_relay_mount(maping_Mount.c_str(), addr.c_str(), port, mount.c_str(), user_pwd.c_str());
    }

    return 0;
}

int relay_account_table::add_new_sys_relay_mount(const char *MapingMount, const char *addr, int port, const char *mount, const char *user_pwd)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    // 先判断这个桶是否已经存在
    relay_account_entry search_entry;
    strcpy(search_entry.MapingMount, MapingMount);
    if (HT_FIND(relay_account_ht, _table, &search_entry))
    {
        // 已经存在
        return 1;
    }

    // 创建一个新桶
    relay_account_entry *new_entry = (relay_account_entry *)malloc(sizeof(relay_account_entry));

    strcpy(new_entry->MapingMount, MapingMount);
    strcpy(new_entry->addr, addr);
    new_entry->port = port;
    strcpy(new_entry->path, mount);
    new_entry->account_num = 0;
    new_entry->account_list = nullptr;
    new_entry->account_last = nullptr;

    unique_account *insert_account = new unique_account;
    strcpy(insert_account->userName_pwd, user_pwd);


    std::string unencode_userID = insert_account->userName_pwd;
    std::string encode_userID = util_base64_encode(unencode_userID);
    strcpy(insert_account->Base64_userID, encode_userID.c_str());

    insert_account->state = 0;

    new_entry->sys_relay_account = insert_account;

    HT_INSERT(relay_account_ht, _table, new_entry);

    return 0;
}

relay_account_entry *relay_account_table::find_idle_sys_relay_account_by_state()
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif
    relay_account_entry **entry;

    if (!(entry = HT_START(relay_account_ht, _table)))
    {
        return nullptr;
    }

    do
    {
        if ((*entry)->sys_relay_account && (*entry)->sys_relay_account->state == 0)
        {
            (*entry)->sys_relay_account->state = 1;
            _num_of_using_user_account++;
            return (*entry);
        }
    } while (entry = HT_NEXT(relay_account_ht, _table, entry));

    return nullptr;
}

// int relay_account_table::add_new_account(char *MapingMount, relay_account *account)
// {
//     // 唯一锁定
// #ifdef ENABLE_TABLE_MUTEX
//     std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
// #endif
//     // 账号传入，由内部分配内存？账号传出，由谁来释放？一般来说就不释放了

//     // 找到指定的桶
//     relay_account_entry *find_entry, search_entry;

//     strcpy(search_entry.MapingMount, MapingMount);
//     if (find_entry = HT_FIND(relay_account_ht, _table, &search_entry))
//     {
//         relay_account *insert_account = (relay_account *)malloc(sizeof(relay_account));
//         memcpy(insert_account, account, sizeof(relay_account));
//         insert_account->state = 0;
//         insert_account->next = nullptr;

//         find_entry->account_last->next = insert_account;
//         find_entry->account_last = insert_account;
//         find_entry->account_num++;

//         return 0;
//     }
//     else
//     {
//         return 1;
//     }
// }

// int relay_account_table::add_new_account(const char *MapingMount, const char *user_pwd)
// {
//     // 唯一锁定
// #ifdef ENABLE_TABLE_MUTEX
//     std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
// #endif
//     // 账号传入，由内部分配内存？账号传出，由谁来释放？一般来说就不释放了

//     // 找到指定的桶
//     relay_account_entry *find_entry, search_entry;

//     strcpy(search_entry.MapingMount, MapingMount);
//     if (find_entry = HT_FIND(relay_account_ht, _table, &search_entry))
//     {
//         relay_account *insert_account = (relay_account *)malloc(sizeof(relay_account));
//         strcpy(insert_account->userName_pwd, user_pwd);

//         char encode_userID[64] = {'\0'};
//         util_base64_encode(encode_userID, insert_account->userName_pwd);

//         strcpy(insert_account->Base64_userID, encode_userID);

//         insert_account->state = 0;
//         insert_account->next = nullptr;

//         if (find_entry->account_list == NULL) // 第一次插入
//         {
//             find_entry->account_last = find_entry->account_list = insert_account;
//         }
//         else
//         {
//             find_entry->account_last->next = insert_account;
//             find_entry->account_last = insert_account;
//         }

//         find_entry->account_num++;

//         _sum_of_user_account++;
//         return 0;
//     }
//     else
//     {
//         return 1;
//     }
// }

int relay_account_table::add_new_maping_mount(const char *MapingMount, const char *addr, int port, const char *mount)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    // 先判断这个桶是否已经存在
    relay_account_entry search_entry;
    strcpy(search_entry.MapingMount, MapingMount);
    if (HT_FIND(relay_account_ht, _table, &search_entry))
    {
        // 已经存在
        return 1;
    }

    // 创建一个新桶
    relay_account_entry *new_entry = (relay_account_entry *)malloc(sizeof(relay_account_entry));

    strcpy(new_entry->MapingMount, MapingMount);
    strcpy(new_entry->addr, addr);
    new_entry->port = port;
    strcpy(new_entry->path, mount);
    new_entry->account_num = 0;
    new_entry->account_list = nullptr;
    new_entry->account_last = nullptr;
    new_entry->sys_relay_account = nullptr;

    HT_INSERT(relay_account_ht, _table, new_entry);

    Maping_Mount_list.insert(MapingMount);

    return 0;
}

// int relay_account_table::maping_mount_is_exist(char *MapingMount)
// {
//     relay_account_entry search_entry;

//     strcpy(search_entry.MapingMount, MapingMount);

//     if (HT_FIND(relay_account_ht, _table, &search_entry))
//     {
//         return 0;
//     }

//     return 1;
// }

int relay_account_table::search_Maping_Mount_list(char *MapingMount)
{
    auto find = Maping_Mount_list.find(MapingMount);

    if (find == Maping_Mount_list.end())
    {
        // 没有找到
        return 1;
    }
    return 0;
}

unique_account *relay_account_table::create_unique_account(const char *user_pwd)
{

    unique_account *account = new unique_account;

    strcpy(account->userName_pwd, user_pwd);

    //util_base64_encode(account->Base64_userID, account->userName_pwd);

    account->state = 0;

    return account;
}

int relay_account_table::add_unique_account(const char *MapingMount, unique_account *account)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif
    // 账号传入，由内部分配内存？账号传出，由谁来释放？一般来说就不释放了

    // 找到指定的桶
    relay_account_entry *find_entry, search_entry;

    strcpy(search_entry.MapingMount, MapingMount);
    if (find_entry = HT_FIND(relay_account_ht, _table, &search_entry))
    {
        relay_account *insert_account = (relay_account *)malloc(sizeof(relay_account));

        insert_account->account = account;
        insert_account->next = nullptr;

        if (find_entry->account_list == NULL) // 第一次插入
        {
            find_entry->account_last = find_entry->account_list = insert_account;
        }
        else
        {
            find_entry->account_last->next = insert_account;
            find_entry->account_last = insert_account;
        }

        find_entry->account_num++;

        _sum_of_user_account++;
        return 0;
    }
    else
    {
        return 1;
    }

    return 0;
}

// int relay_account_table::update_Maping_Mount_list(char *MapingMount)
// {
//     // 唯一锁定
// #ifdef ENABLE_TABLE_MUTEX
//     std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
// #endif

//     return 0;
// }

relay_account_entry *relay_account_table::find_match_MapingMonut(char *MapingMount)
{
    // 共享锁定
#ifdef ENABLE_TABLE_MUTEX
    std::shared_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    relay_account_entry find_entry;
    strcpy(find_entry.MapingMount, MapingMount);

    relay_account_entry *match_entry;

    return HT_FIND(relay_account_ht, _table, &find_entry);
}

unique_account *relay_account_table::find_idle_account_by_MapingMount(char *MapingMount)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    relay_account_entry find_entry;
    strcpy(find_entry.MapingMount, MapingMount);

    relay_account_entry *match_entry;

    match_entry = HT_FIND(relay_account_ht, _table, &find_entry);

    if (!match_entry)
    {
        return NULL;
    }

    for (int i = 0; i < match_entry->account_num; i++)
    {
        // 放到尾部
        match_entry->account_last->next = match_entry->account_list;
        match_entry->account_last = match_entry->account_list;
        // 移动头部
        match_entry->account_list = match_entry->account_list->next;
        // 置空尾指针
        match_entry->account_last->next = NULL;

        if (match_entry->account_last->account->state == 0)
        {
            match_entry->account_last->account->state = 1;
            return match_entry->account_last->account;
        }
    }

    // 循环一圈，也没有找到可用账号，那就返回空
    return NULL;
}

int relay_account_table::give_back_account(unique_account *using_account)
{
    // 唯一锁定
#ifdef ENABLE_TABLE_MUTEX
    std::unique_lock<std::shared_mutex> lck(_lock); // 执行mutex_.lock();
#endif

    using_account->state = 0;
    _num_of_using_user_account--;

    return 0;
}
