#include <iostream>

#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include <event2/listener.h>
#include "spdlog/spdlog.h"

#include <arpa/inet.h>

#include <unordered_map>

#include "tcpsvr_loger.h"

int main(int argc, char **argv)
{
    // 11005 10005

    if (argc < 3)
    {
        return 1;
    }

    tcpsvr_loger loger(atoi(argv[1]), atoi(argv[2]));

    loger.start();

    return 0;
}