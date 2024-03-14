#include <iostream>
#include <string>

#include "hiredis.h"

// #include "rtklib.h"
#include <unistd.h>

// #include "spdlog/spdlog.h"

#include <event2/event.h>
#include <event2/thread.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#include <sys/prctl.h>
#include <sys/resource.h>

#include "redismq.h"

#include "MsgSwitch.h"

extern void sleepms(int ms)
{
#ifdef WIN32
    if (ms < 5)
        Sleep(1);
    else
        Sleep(ms);
#else
    struct timespec ts;
    if (ms <= 0)
        return;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)(ms % 1000 * 1000000);
    nanosleep(&ts, NULL);
#endif
}

extern uint32_t tickget(void)
{
#ifdef WIN32
    return (uint32_t)timeGetTime();
#else
    struct timespec tp = {0};
    struct timeval tv = {0};

#ifdef CLOCK_MONOTONIC_RAW
    /* linux kernel > 2.6.28 */
    if (!clock_gettime(CLOCK_MONOTONIC_RAW, &tp))
    {
        return tp.tv_sec * 1000u + tp.tv_nsec / 1000000u;
    }
    else
    {
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000u + tv.tv_usec / 1000u;
    }
#else
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000u + tv.tv_usec / 1000u;
#endif
#endif /* WIN32 */
}

std::string util_random_string(int string_len)
{
    std::string rand_str;

    auto now = std::chrono::system_clock::now();
    auto time = now.time_since_epoch();
    unsigned int seed = time.count();

    for (int i = 0; i < string_len; i++)
    {

        switch (rand_r(&seed) % 3)
        {
        case 0:
            rand_str += rand_r(&seed) % 26 + 'a';
            break;
        case 1:
            rand_str += rand_r(&seed) % 26 + 'A';
            break;
        case 2:
            rand_str += rand_r(&seed) % 10 + '0';
            break;

        default:
            break;
        }
    }

    return rand_str;
}

int print_data(const char *data)
{
    // printf("%s\n", data);

    return 0;
}

static void *pub_thread(void *arg)
{
    auto para = static_cast<std::pair<redis_mq *, int> *>(arg);

    auto size = para->second;
    auto x = para->first;

    std::string channal = util_random_string(10);

    x->add_sub_queue(channal.c_str(), print_data);

    std::string data = util_random_string(size);
    while (1)
    {
        x->add_pub_queue(channal.c_str(), data.c_str());
        sleep(1);
    }

    // Unsubscribe(channal.c_str(), channal.c_str());
}

int main()
{

    // 启用dump
    prctl(PR_SET_DUMPABLE, 1);

    // 设置core dunp大小
    struct rlimit rlimit_core;
    rlimit_core.rlim_cur = RLIM_INFINITY; // 设置大小为无限
    rlimit_core.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlimit_core);

    redis_mq *x = new redis_mq();

    x->init("127.0.0.1", 16379, "snLW_1234");
    x->start();

    int length = 200000;


    std::string channal = util_random_string(10);

    x->add_sub_queue(channal.c_str(), print_data);

    std::string data = util_random_string(length);
    while (1)
    {
        x->add_pub_queue(channal.c_str(), data.c_str());
        sleep(1);
    }


    // for (int i = 0; i < 1; i++)
    // {
    //     pthread_t id;

    //     auto arg = new std::pair<redis_mq *, int>(x, length);

    //     int ret = pthread_create(&id, NULL, pub_thread, arg);

    //     // sleep(1);
    // }

    // std::string redis_ip = "127.0.0.1";
    // int redis_port = 16379;
    // std::string redis_requirepass = "snLW_1234";

    // pthread_t id;
    // int ret1 = pthread_create(&id, NULL, libevent_thread, );
    // if (ret1)
    // {
    //     return -1;
    // }

    // redisContext *_noblock_context;

    // _noblock_context = redisConnect(redis_ip.c_str(), redis_port);
    // //_noblock_context = redisConnect(redis_ip.c_str(), redis_port);

    // auto reply = (redisReply *)redisCommand(_noblock_context, "AUTH %s", redis_requirepass.c_str());

    // while (1)
    // {
    //     redisAppendCommand(_noblock_context, "PUBLISH %s %s", redis_requirepass.c_str(), redis_requirepass.c_str());

    //     sleepms(1000);
    //     redisAppendCommand(_noblock_context, "PUBLISH %s %s", redis_requirepass.c_str(), redis_requirepass.c_str());
    //     sleepms(1000);

    //     redisGetReply(_noblock_context, (void **)&reply); // reply for SET
    //     freeReplyObject(reply);
    //     redisGetReply(_noblock_context, (void **)&reply); // reply for GET
    //     freeReplyObject(reply);

    //     sleepms(1000);

    //     // auto in1 = redisAppendCommand(_noblock_context, "PUBLISH %s %s", redis_requirepass.c_str(), redis_requirepass.c_str());
    //     // auto in2 = redisAppendCommand(_noblock_context, "SET %s %s", redis_requirepass.c_str(), redis_requirepass.c_str());
    //     // sleepms(1000);
    // }

    return 0;
}
