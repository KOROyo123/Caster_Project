#include "relay_account_tb.h"

relay_account_tb::relay_account_tb()
{
}

relay_account_tb::~relay_account_tb()
{
}

/// @brief 读取第三方转发配置文件
/// @param conf_path 配置文件的路径
/// @return
int relay_account_tb::load_account_file(std::string conf_path)
{
    FILE *listfp = fopen(conf_path.c_str(), "r");
    if (listfp == NULL)
    {
        spdlog::warn("[relay_account_tb]: load conf file fail! can't open file: {}", conf_path);
        return false;
    }
    std::ifstream f(conf_path);
    json data = json::parse(f);

    //  读取用户转发配置
    if (data["user_config"].is_null())
    {
        spdlog::warn("[relay_account_tb]: can't find item: [user_config]", conf_path);
    }
    else
    {
        for (int i = 0; i < data["user_config"].size(); i++)
        {
            json one_conf = data["user_config"].at(i);
            load_group_conf(one_conf);
        }
    }

    // 读取系统转发配置
    if (data["sys_config"].is_null())
    {
        spdlog::warn("[relay_account_tb]: can't find item: [sys_config]", conf_path);
    }
    else
    {
        load_sys_relay_conf(data["sys_config"]);
    }

    return 0;
}

std::set<std::string> relay_account_tb::get_sys_mpt()
{
    return _sys_relay_mpt;
}

std::set<std::string> relay_account_tb::get_usr_mpt()
{
    return _usr_relay_mpt;
}

/// @brief 找到一个映射挂载点的空闲账号，将其从空闲账号表移动到已使用账号表，并返回账号名和密码
/// @param mount_point 要找寻的挂载点
/// @return 账号名密码（用于去已使用账号表中找到对应的连接参数）
std::string relay_account_tb::find_trd_idel_account(std::string mount_point)
{
    // 找_account_map

    for (auto iter : _usr_relay_map)
    {
        auto item = iter.second.find(mount_point);

        if (item != iter.second.end())
        {
            std::string userid = iter.first;

            _usr_using_map.insert(iter);
            _usr_relay_map.erase(iter.first);

            return userid;
        }
    }

    return std::string();
}

/// @brief 从已使用账号表中，查询具体的第三方挂载点连接信息
/// @param user_id 获取的账号密码
/// @param mount_point 要连接的挂载点
/// @return 对应挂载点连接第三方的信息
json relay_account_tb::get_trd_account_info(std::string user_id, std::string mount_point)
{

    auto item = _usr_using_map.find(user_id);
    if (item == _usr_using_map.end())
    {
        spdlog::error("[relay_account_tb]: {} fail, user account :{}.", __func__, user_id);
        return json();
    }

    auto mp_info = item->second.find(mount_point);
    if (mp_info == item->second.end())
    {
        spdlog::error("[relay_account_tb]: {} fail, mount point :{}.", __func__, mount_point);
        return json();
    }

    return mp_info->second;
}

/// @brief 将不使用的账号重新移动到空闲账号表
/// @param user_id 使用的账号密码
/// @return
int relay_account_tb::give_back_usr_account(std::string user_id)
{

    auto item = _usr_using_map.find(user_id);
    if (item == _usr_using_map.end())
    {
        spdlog::error("[relay_account_tb]: {} fail, user account :{}.", __func__, user_id);
        return 1;
    }

    _usr_relay_map.insert(*item);
    _usr_using_map.erase(user_id);

    return 0;
}

std::string relay_account_tb::find_sys_idel_account(std::string mount_point)
{
    return std::string();
}

json relay_account_tb::get_sys_account_info(std::string user_id, std::string mount_point)
{
    return json();
}

int relay_account_tb::give_back_sys_account(std::string user_id)
{
    auto item = _sys_using_map.find(user_id);
    if (item == _sys_using_map.end())
    {
        spdlog::error("[relay_account_tb]: {} fail, user account :{}.", __func__, user_id);
        return 1;
    }

    _sys_relay_map.insert(*item);
    _sys_using_map.erase(user_id);

    return 0;
}

/// @brief 读取配置文件中的一组用户配置
/// @param group_conf json格式的配置文件中的一组用户配置
/// @return
int relay_account_tb::load_group_conf(json group_conf)
{

    for (int i = 0; i < group_conf["username_pwd"].size(); i++)
    {
        std::string user_pwd;
        user_pwd = group_conf["username_pwd"].at(i);

        std::unordered_map<std::string, json> mount_maping;

        for (int j = 0; j < group_conf["Maping"].size(); j++)
        {
            json maping_mp = group_conf["Maping"].at(j);
            mount_maping.insert(std::pair(maping_mp["Maping_Mount"], maping_mp));
            _usr_relay_mpt.insert(maping_mp["Maping_Mount"]);
        }
        _usr_relay_map.insert(std::pair(user_pwd, mount_maping));
    }

    spdlog::info("[relay_account_tb]: Load [{}] user relay account.", _usr_relay_map.size());

    return 0;
}

/// @brief 读取配置文件中的系统转发配置
/// @param sys_relay_conf json格式的配置文件中的系统转发配置
/// @return
int relay_account_tb::load_sys_relay_conf(json sys_relay_conf)
{

    for (int i = 0; i < sys_relay_conf.size(); i++)
    {
        json sys_relay_mp = sys_relay_conf.at(i);
        _usr_relay_mpt.insert(sys_relay_mp["Local_Mount"]);
        _sys_relay_map.insert(std::pair(sys_relay_mp["Local_Mount"], sys_relay_mp));
    }

    spdlog::info("[relay_account_tb]: Load [{}] sys relay task.", _sys_relay_map.size());

    return 0;
}
