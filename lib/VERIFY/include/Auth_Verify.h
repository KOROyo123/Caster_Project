#pragma once
#include <event2/event.h>

#define AUTH_REPLY_ERR -1
#define AUTH_REPLY_OK 0

#define AUTH_REPLY_STRING 1
#define AUTH_REPLY_ARRAY 2
#define AUTH_REPLY_INTEGER 3
#define AUTH_REPLY_NIL 4

struct AuthReply
{
    int type;
    char *str;
    size_t str_len;
    int integer;
};

typedef void (*VerifyCallback)(void *arg, AuthReply *reply);

namespace AUTH
{
    enum Auth_type
    {
        AUTH_COMMON = 1,
        AUTH_STATION,
        AUTH_CLIENT
    };

    int Init(const char *json_conf, event_base *base);
    int Free();

    int Clear();

    // 验证密码是否通过，返回有效期，或登录失败
    int Verify(const char *userID, VerifyCallback cb, void *arg, Auth_type type = AUTH_COMMON);

    // 添加登录记录（无论是否成功都会添加一条登录记录），登录成功，添加到在线表，登录失败，返回登录失败
    int Add_Login_Record(const char *userID, const char *connect_key, VerifyCallback cb, void *arg, Auth_type type = AUTH_COMMON);

    // 添加登出记录，删除在线表记录
    int Add_Logout_Record(const char *userID, const char *connect_key, Auth_type type = AUTH_COMMON);

    // 添加账号
    int Add_Account_Item();
    // 获取账号信息
    int Get_Account_Item();
    // 设置账号信息
    int Set_Account_Item();
    // 删除账号信息
    int Del_Account_Item();

} // namespace CASTER

// Redis 内部维护表
// 一张
