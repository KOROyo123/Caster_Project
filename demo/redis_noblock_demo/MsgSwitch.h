// #pragma once

//  int Init(const char* groupid, const char* szIP = (char*)"127.0.0.1", int port = 5672);

//  int PostMsg(const char* moduleId, const char* dataId, const char* szdata);

//  int PostMsg1(const char* moduleId, int dataId, const char* szdata);

//  int RegisterNotify(const char* moduleId, const char* dataId, int(*pFunc)(const char* szdata));

//  int RegisterNotify1(const char* moduleId, int dataId, int(*pFunc)(const char* szdata));

//  int ShutDown();

//  int StartMQ();

/*******************************************************************************
 *   文件名称 : MsgSwitch.h
 *   作    者 : 王全宝
 *   创建时间 : 2013/05/15  16:31
 *   文件描述 : 消息交互接口声明
 *   版权声明 : Copyright (C) 2012-2013 联迪恒星（南京）信息系统有限公司
 *   修改历史 : 2013/05/15 1.00 初始版本
 *******************************************************************************/
#define MSGSWITCH_API __attribute__((visibility("default")))

/*******************************************************************************
 * 函数名称  : Init
 * 函数描述  : 初始模块
 * 输入参数  : moduleId： 模块ID
 * 输出参数  : N/A
 * 返回值    : int  -1: fail
 * 备注      : N/A
 * 修改日期     修改人   修改内容
 * -----------------------------------------------
 * 2013/05/15    王全宝   新建
 *******************************************************************************/
// 64
extern "C" MSGSWITCH_API int Init(const char *groupid, const char *szIP = (char *)"127.0.0.1", int port = 6379);
extern "C" MSGSWITCH_API int Init1(const char* groupid, const char* szIP, int port,const char *pwd);
/*******************************************************************************
 * 函数名称  : PostMsg
 * 函数描述  : 发送消息
 * 输入参数  : moduleId ：模块ID；0xFFFF为广播消息专用ID;
 *            dataId: 数据ID；   0xFF00 -- 0xFFFE为广播消息专用ID; 0xFFFF为消息循环退出；
 *            szdata: 数据串
 * 输出参数  : N/A
 * 返回值    : int -:fail
 * 备注      : N/A
 * 修改日期     修改人   修改内容
 * -----------------------------------------------
 * 2013/05/15    王全宝   新建
 *******************************************************************************/
extern "C" MSGSWITCH_API int PostMsg(const char *moduleId, const char *dataId, const char *szdata);

extern "C" MSGSWITCH_API int PostMsg1(const char *moduleId, int dataId, const char *szdata);

/*******************************************************************************
 * 函数名称  : RegisterNotify
 * 函数描述  : 注册回调处理函数
 * 输入参数  : dataId：数据id
 *             pFunc: 函数指针
 * 输出参数  : N/A
 * 返回值    : int -:fail
 * 备注      : N/A
 * 修改日期     修改人   修改内容
 * -----------------------------------------------
 * 2013/05/15    王全宝   新建
 *******************************************************************************/
extern "C" MSGSWITCH_API int RegisterNotify(const char *moduleId, const char *dataId, int (*pFunc)(const char *szdata));

extern "C" MSGSWITCH_API int RegisterNotify1(const char *moduleId, int dataId, int (*pFunc)(const char *szdata));

// extern "C" MSGSWITCH_API int Unsubscribe(const char *moduleId, const char *dataId);

// extern "C" MSGSWITCH_API int Unsubscribe1(const char *moduleId, int dataId);

/*******************************************************************************
 * 函数名称  : ShutDown
 * 函数描述  : 模块注销
 * 输入参数  : N/A
 * 输出参数  : N/A
 * 返回值    : int -:fail
 * 备注      : N/A
 * 修改日期     修改人   修改内容
 * -----------------------------------------------
 * 2013/05/15    王全宝   新建
 *******************************************************************************/
extern "C" MSGSWITCH_API int ShutDown();

extern "C" MSGSWITCH_API int StartMQ();


extern "C" MSGSWITCH_API int Unsubscribe(const char *moduleId, const char *dataId);

extern "C" MSGSWITCH_API int Unsubscribe1(const char *moduleId, int dataId);


extern "C" MSGSWITCH_API int SetValue(const char *moduleId, const char *dataId, const char *szdata);

extern "C" MSGSWITCH_API int SetValue1(const char *moduleId, int dataId, const char *szdata);