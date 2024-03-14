#include <iostream>

#include "MsgSwitch.h"

#include "spdlog/spdlog.h"
#include "rtklib.h"

int recv1(const char *data)
{
    spdlog::info("{}", data);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        int port;
        char ip[64] = {'\0'};
        sscanf(argv[1], "%s", ip);
        sscanf(argv[2], "%d", &port);

        Init("null", ip, port);
    }
    else
    {
        Init("null", "127.0.0.1", 6379);
    }
    //Init("null", "127.0.0.1", 6379);

    RegisterNotify("test", "111", recv1);

    RegisterNotify("test", "222", recv1);

    StartMQ();

    while (1)
    {
        PostMsg("test", "111", "xxxxx");

        PostMsg("test", "222", "bbbb");

        sleepms(1000);
    }

    ShutDown();

    return 0;
}