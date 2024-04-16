#include "Auth_Verify.h"
#include <spdlog/spdlog.h>

#include "auth_verify_internal.h"

redis_auth_internal *auth_svr = nullptr;

int AUTH::Init(const char *json_conf, event_base *base)
{
    return 0;
}

int AUTH::Free()
{
    return 0;
}

int AUTH::Clear()
{
    return 0;
}

int AUTH::Verify(const char *userID, VerifyCallback cb, void *arg, Auth_type type)
{
    AuthReply reply;
    reply.type = AUTH_REPLY_OK;
    cb(nullptr, arg, &reply);
    return 0;
}

int AUTH::Add_Login_Record(const char *user_name, const char *connect_key, VerifyCallback cb, void *arg, Auth_type type)
{
    AuthReply reply;
    reply.type = AUTH_REPLY_OK;
    cb(nullptr, arg, &reply);
    return 0;
}

int AUTH::Add_Logout_Record(const char *user_name, const char *connect_key, Auth_type type)
{
    return 0;
}

int AUTH::Add_Account_Item()
{
    return 0;
}

int AUTH::Get_Account_Item()
{
    return 0;
}

int AUTH::Set_Account_Item()
{
    return 0;
}

int AUTH::Del_Account_Item()
{
    return 0;
}
