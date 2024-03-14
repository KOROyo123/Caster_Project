#pragma once

#include "smtp_ssl.h"

#include <condition_variable>

class smtp_email
{
private:

    int _state=0;

    std::string _from;
    std::string _passs;
    std::string _to;
    // 周期消息
    std::string _period_msg;
    // 即时消息
    std::string _temp_msg;

    /* data */
    std::string _host;
	std::string _port;
    SmtpEmail *_sender = nullptr;

    int _Intv;
    // 锁和条件变量
    std::mutex _lock;
    std::condition_variable _cv;

public:
    smtp_email(/* args */);
    ~smtp_email();

    int load_conf(const char *path);

    int start();

    int set_period_msg(const char *msg);
    int send_temp_msg(const char *msg);

    int active_stmp_thread();

    int transfer_email_now();

    static void* stmp_thread(void *arg);
    int cal_wait_time();
    int thread_waiting(int waiting_sec);
};
