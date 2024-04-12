#define __class__ "redis_msg_internal"

#include <string>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class redis_msg_internal
{
private:
    std::string _redis_IP;
    int _redis_port;
    std::string _redis_Requirepass;

    event_base *_base;

public:
    redisAsyncContext *_pub_context;
    redisAsyncContext *_sub_context;

public:
    redis_msg_internal(json conf, event_base *base);
    ~redis_msg_internal();

    int start();
    int stop();

    // Redis回调
    static void Redis_Connect_Cb(const redisAsyncContext *c, int status);
    static void Redis_Disconnect_Cb(const redisAsyncContext *c, int status);
};
