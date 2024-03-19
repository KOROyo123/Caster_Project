#include <iostream>
#include <unordered_map>
#include "knt/knt.h"
#include <spdlog/spdlog.h>
#include "event.h"
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"

event_base *base;

struct data_t
{
    int i = 0;
    char x[4096];
};

class map
{
private:
    /* data */

    std::unordered_map<std::string, data_t *> _map;
    std::unordered_map<int, int> _map1;

    event_base *_base;

    std::unordered_map<std::string, bufferevent *> _map2;

    event *_timeout_ev;
    timeval _timeout_tv;

public:
    map(/* args */);
    ~map();

    int start();

    int test();

    int insert(int i);
    int remove(int i);

    int insert1(int i);
    int remove1(int i);

    int insert2(int i);
    int remove2(int i);

    static void TimeoutCallback(evutil_socket_t fd, short events, void *arg);
};

map::map(/* args */)
{

    _base = event_base_new();
}

map::~map()
{
}

int map::insert(int i)
{
    std::string key = util_random_string(8);
    data_t *data = new data_t;
    _map.insert(std::pair(key, data));
    return 0;
}

int map::remove(int i)
{
    if (_map.size() > 0)
    {
        auto item = _map.begin();
        auto data = item->second;
        delete data;
        _map.erase(item->first);
    }
    return 0;
}

int map::insert1(int i)
{
    _map1.insert(std::pair(i, 2 * i));
    return 0;
}

int map::start()
{

    _timeout_tv.tv_sec = 10;
    _timeout_tv.tv_usec = 0;

    _timeout_ev = event_new(_base, -1, EV_PERSIST, TimeoutCallback, this);
    event_add(_timeout_ev, &_timeout_tv);

    event_base_dispatch(_base);

    return 0;
}

void map::TimeoutCallback(evutil_socket_t fd, short events, void *arg)
{
    auto *svr = static_cast<map *>(arg);

    svr->test();
}

int map::remove1(int i)
{
    _map1.erase(i);
    return 0;
}

int map::test()
{
    for (int j = 0; j < 40; j++)
    {
        spdlog::info("insert before:{}", util_get_use_memory());
        for (int i = 0; i < 5000; i++)
        {
            insert2(i + j * 10000);
        }
        spdlog::info("insert after:{}", util_get_use_memory());

        spdlog::info("remove  before:{}", util_get_use_memory());

        for (int i = 0; i < 5000; i++)
        {
            remove2(i + j * 10000);
        }

        spdlog::info("remove after:{}", util_get_use_memory());
    }
    return 0;
}

int map::insert2(int i)
{
    std::string key = util_random_string(8);

    // bufferevent **bevs[2];

    // bufferevent_pair_new(_base, BEV_OPT_CLOSE_ON_FREE, *bevs);

    // std::string key1 = util_random_string(8);

    // _map2.insert(std::pair(key1, &bevs));

    return 0;
}
int map::remove2(int i)
{

    if (_map2.size() > 0)
    {
        auto item = _map2.begin();

        auto data = item->second;

        bufferevent_free(data);

        _map.erase(item->first);
    }
    return 0;
}

int main()
{

    // 创建一个map
    map m;

    m.start();

    // 随机插入元素

    // 显示内存

    return 0;
}