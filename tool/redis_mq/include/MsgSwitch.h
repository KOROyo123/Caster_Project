#pragma once



 int Init(const char* groupid, const char* szIP = (char*)"127.0.0.1", int port = 5672);

 int PostMsg(const char* moduleId, const char* dataId, const char* szdata);

 int PostMsg1(const char* moduleId, int dataId, const char* szdata);


 int RegisterNotify(const char* moduleId, const char* dataId, int(*pFunc)(const char* szdata));

 int RegisterNotify1(const char* moduleId, int dataId, int(*pFunc)(const char* szdata));


 int ShutDown();

 int StartMQ();

