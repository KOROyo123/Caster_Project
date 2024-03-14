#pragma once
#include <iostream>
#include <fstream>
#include <mutex> //unique_lock
#include <shared_mutex>
#include <set>
#include <string.h>

#include <unordered_map>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 声明哈希表结构
//  定义哈希表的类型名和存储的元素

class relay_account_tb
{
private:
    std::set<std::string> _sys_relay_mpt;
    std::set<std::string> _usr_relay_mpt;

    std::unordered_map<std::string, std::unordered_map<std::string, json>> _usr_relay_map;
    std::unordered_map<std::string, std::unordered_map<std::string, json>> _usr_using_map;
    std::unordered_map<std::string, json> _sys_relay_map;
    std::unordered_map<std::string, json> _sys_using_map;

    // 统计信息
    int _sum_of_user_account = 0;
    int _num_of_using_user_account = 0;

    int _sum_of_sys_relay_account = 0;
    int _sum_of_using_user_account = 0;

    /* data */
public:
    relay_account_tb();
    ~relay_account_tb();

    int load_account_file(std::string conf_path);

    std::set<std::string> get_sys_mpt();
    std::set<std::string> get_usr_mpt();

    std::string find_trd_idel_account(std::string mount_point);
    json get_trd_account_info(std::string user_id, std::string mount_point);
    int give_back_usr_account(std::string user_id);

    std::string find_sys_idel_account(std::string mount_point);
    json get_sys_account_info(std::string user_id, std::string mount_point);
    int give_back_sys_account(std::string user_id);

private:
    int load_group_conf(json group_conf);
    int load_sys_relay_conf(json sys_relay_conf);

    
};
