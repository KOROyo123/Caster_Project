#include "MsgSwitch.h"

#include "redismq.h"

redis_mq *redismq;

MSGSWITCH_API int Init(const char *groupid, const char *szIP, int port)
{
    redismq = new redis_mq();
    // redismq->Init(szIP, port, "NaviCloud1234");
    redismq->init(szIP, port, "snLW_1234");

    return 0;
}

MSGSWITCH_API int Init1(const char *groupid, const char *szIP, int port, const char *pwd)
{
    redismq = new redis_mq();
    redismq->init(szIP, port, pwd);

    return 0;
}

MSGSWITCH_API int PostMsg(const char *moduleId, const char *dataId, const char *szdata)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq->add_pub_queue(channel.c_str(), szdata);

    return 0;
}

MSGSWITCH_API int PostMsg1(const char *moduleId, int dataId, const char *szdata)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq->add_pub_queue(channel.c_str(), szdata);
    return 0;
}

MSGSWITCH_API int RegisterNotify(const char *moduleId, const char *dataId, int (*pFunc)(const char *szdata))
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq->add_sub_queue(channel.c_str(), pFunc);

    return 0;
}

MSGSWITCH_API int RegisterNotify1(const char *moduleId, int dataId, int (*pFunc)(const char *szdata))
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq->add_sub_queue(channel.c_str(), pFunc);

    return 0;
}

MSGSWITCH_API int Unsubscribe(const char *moduleId, const char *dataId)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq->add_cal_queue(channel.c_str());

    return 0;
}

MSGSWITCH_API int Unsubscribe1(const char *moduleId, int dataId)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq->add_cal_queue(channel.c_str());

    return 0;
}

MSGSWITCH_API int SetValue(const char *moduleId, const char *dataId, const char *szdata)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += dataId;

    redismq->add_set_queue(channel.c_str(), szdata);
    return 0;
}

MSGSWITCH_API int SetValue1(const char *moduleId, int dataId, const char *szdata)
{
    std::string channel;

    channel += moduleId;
    channel += "_";
    channel += std::to_string(dataId);

    redismq->add_set_queue(channel.c_str(), szdata);
    return 0;
}

MSGSWITCH_API int ShutDown()
{
    if (redismq == nullptr)
    {
        return 0;
    }

    redismq->stop();
    return 0;
}

MSGSWITCH_API int StartMQ()
{
   redismq->start();
    return 0;
}
