#define __class__ "redis_msg_internal"

#include <string>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class redis_auth_internal
{
private:
    /* data */
public:
    redis_auth_internal(/* args */);
    ~redis_auth_internal();
};


