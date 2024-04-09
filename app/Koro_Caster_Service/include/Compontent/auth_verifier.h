#pragma once
#include <event2/event.h>

typedef void (*verifycallback)(void *arg, const char *auth_info, size_t info_length);

namespace AUTH
{
    int Init();
    int Free(); 

    int Verify_Server_UserID();
    int Verify_Client_UserID();

}