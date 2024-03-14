#include "smtp_email.h"
#include <cstring>

int char2Arg(char *str, int *argc, char **argv, int number)
{
    char *p;
    int num = 0;
    int word_start = 1;

    if (argc == NULL || argv == NULL)
        return -1;

    p = str;

    while (*p)
    {
        if ((*p == '\r') || (*p == '\n'))
        {
            *p = '\0';
            break;
        }
        if ((*p == ' ') || (*p == '\t'))
        {
            word_start = 1;
            *p = '\0';
            p++;
            continue;
        }
        if (num >= number)
            break;

        if (word_start)
        {
            argv[num++] = p;
            word_start = 0;
        }
        p++;
    }

    *argc = num;

    return 0;
}

smtp_email::smtp_email(/* args */)
{
}

smtp_email::~smtp_email()
{
}

int smtp_email::load_conf(const char *path)
{

    // 读取配置文件
    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        _state = 0;
        return -1;
        // 配置文件打开失败，要关闭线程
    }

    char line[1024];

    char host[256], port[256], user[256], pwd[256], recv[256];

    int itev = 86400;

    while (fgets(line, 1024, fp))
    {
        if (strstr(line, "#") != NULL) // 注释行 忽略
        {
            continue;
        }

        int argc;
        char *argv[10];
        char2Arg(line, &argc, argv, 10);

        strcpy(host, argv[0]); // 服务地址
        strcpy(port, argv[1]); // 服务端口
        strcpy(user, argv[2]); // 用户名
        strcpy(pwd, argv[3]);  // 密码
        strcpy(recv, argv[4]); // 收件人
        itev = atoi(argv[5]);

        break;
    }

    _host = host;
    _port = port;
    _from = user;
    _passs = pwd;
    _to = recv;
    _Intv = itev;

    // _host = "smtp.163.com";
    // _port = "25";
    // _from = "rhg171281@163.com";
    // _passs = "VAPSCMFTFYSDRICL";
    // _to = "1121910079@qq.com";

    // _Intv = 600;

    _state = 1;

    fclose(fp);

    return 0;
}
int smtp_email::start()
{

    if (_state == 0)
    {
        // 缺少配置文件
        return 1;
    }

    _sender = new SmtpEmail(_host, _port);

    pthread_t id;
    int ret = pthread_create(&id, NULL, stmp_thread, this);
    if (ret)
    {
        return 1;
    }
    return 0;
}

int smtp_email::set_period_msg(const char *msg)
{

    _period_msg = msg;

    return 0;
}

int smtp_email::send_temp_msg(const char *msg)
{

    _temp_msg = msg;

    active_stmp_thread();

    return 0;
}

int smtp_email::active_stmp_thread()
{

    _cv.notify_one();

    return 0;
}

int smtp_email::transfer_email_now()
{

    if (_sender == nullptr)
    {
        return 1;
    }

    std::string msg;
    std::string subject;
    if (_temp_msg.size() > 0)
    {
        subject = "系统提示信息";
        msg = _temp_msg + _period_msg;
    }
    else
    {
        subject = "定期统计信息";
        msg = _period_msg;
    }

    if (msg.size() > 0)
    {
        _sender->SendEmail(_from, _passs, _to, subject, msg);
        _temp_msg.clear();
    }

    return 0;
}

void *smtp_email::stmp_thread(void *arg)
{

    auto svr = static_cast<smtp_email *>(arg);

    // 发送一个启动邮件

    // 计算等待时间

    while (1)
    {
        svr->thread_waiting(svr->cal_wait_time());

        svr->transfer_email_now();
    }
}

int smtp_email::cal_wait_time()
{

    // 获取当前时间，取余，得到等待时间
    time_t now = time(0);

    return now % _Intv;
}

int smtp_email::thread_waiting(int waiting_sec)
{
    std::unique_lock<std::mutex> lock(_lock);
    // 因为只有一个邮件线程来处理任务队列（消费者），所以不存在虚假唤醒
    // if (head->cv.wait_for(lock, std::chrono::seconds(waitTime)) == std::cv_status::timeout)
    _cv.wait_for(lock, std::chrono::seconds(waiting_sec));

    return 0;
}
