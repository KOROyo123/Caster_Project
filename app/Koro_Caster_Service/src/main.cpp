#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include "process_queue.h"

#include <sys/prctl.h>
#include <sys/resource.h>

#include <coroutine>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "ntrip_caster.h"

using json = nlohmann::json;

#define CONF_PATH "Koro_Caster_Service_Conf.json"

// #define SOFTWARE_NAME "KORO_Caster"
// #define SOFTWARE_VERSION "0.0.1"

// trace：最详细的日志级别，提供追踪程序执行流程的信息。
// debug：调试级别的日志信息，用于调试程序逻辑和查找问题。
// info：通知级别的日志信息，提供程序运行时的一般信息。
// warn：警告级别的日志信息，表明可能发生错误或不符合预期的情况。
// error：错误级别的日志信息，表明发生了某些错误或异常情况。
// critical：严重错误级别的日志信息，表示一个致命的或不可恢复的错误。

#include "DB/relay_account_tb.h"


// #define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_DEBUG(spdlog::get("daily_logger"), __VA_ARGS__)
// #define LOG(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_INFO(spdlog::get("daily_logger"), __VA_ARGS__)
// #define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_WARN(spdlog::get("daily_logger"), __VA_ARGS__)
// #define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_ERROR(spdlog::get("daily_logger"), __VA_ARGS__)


int main(int argc, char **argv)
{
    //----------------------功能测试区

    // relay_account_tb relay;

    // relay.load_account_file("relay_account.json");

    // std::string account = relay.find_idel_account("CMCC-GRCE");

    // json info = relay.get_account_info(account, "CMCC-GRCE");

    // std::string Maping_Mount = info["Maping_Mount"];
    // std::string addr = info["addr"];
    // int port = info["port"];
    // std::string Mount = info["Mount"];

    // relay.give_back_account(account);

    //---------------------------------
// SPDLOG_DEBUG
    //解析输入：
    std::string conf_path;


    conf_path=CONF_PATH;
    // if (argc > 1)
    // {
    //     conf_path=argv[1];
    // }
    // else
    // {
    //     conf_path=CONF_PATH;
    // }

    // 程序启动
    spdlog::info("{}/{} Start", SOFTWARE_NAME, SOFTWARE_VERSION);
    // 打开配置文件
    spdlog::info("Conf Path:{}", conf_path);
    std::ifstream f(conf_path);

    if (!f.is_open())
    {
        spdlog::error("Conf File Open Fail! Exit.");
        return 0;
    }

    // 读取全局配置
    spdlog::info("Load Conf...");
    json cfg = json::parse(f);
    f.close();


    bool Core_Dump = cfg["Debug_Mode"]["Core_Dump"];
    bool Debug_Info = cfg["Debug_Mode"]["Debug_Info"];
    bool Info_Record = cfg["Debug_Mode"]["Info_Record"];

    if (Core_Dump)
    {
        // 启用dump
        prctl(PR_SET_DUMPABLE, 1);

        // 设置core dunp大小
        struct rlimit rlimit_core;
        rlimit_core.rlim_cur = RLIM_INFINITY; // 设置大小为无限
        rlimit_core.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_CORE, &rlimit_core);
    }


    // char *addr = (char *)0; // 设置 addr 变量为内存地址 "0"
 
    // *addr = '\0';           // 向内存地址 "0" 写入数据
 


    bool logfile=cfg["Log_File_Setting"]["Info_Log"]["Switch"];
    std::string logpath=cfg["Log_File_Setting"]["Info_Log"]["Save_Path"];
    int  file_hour=cfg["Log_File_Setting"]["Info_Log"]["File_Hour"];
    int  file_min=cfg["Log_File_Setting"]["Info_Log"]["File_Min"];

    // 初始化日志系统

    //auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 0, 00);

    std::vector<spdlog::sink_ptr> sinks;
    //输出到控制台
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    //输出到文件
    if(logfile)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(logpath, file_hour, file_min));
    }
    
    //把所有sink放入logger
    auto logger = std::make_shared<spdlog::logger>("log", begin(sinks), end(sinks));
    spdlog::set_default_logger(logger);

    spdlog::flush_every(std::chrono::seconds(5));//设置日志刷新时间
    spdlog::flush_on(spdlog::level::warn);//如果遇到警告以上的信息则立即刷新日志，避免丢失记录

    //spdlog::set_level(spdlog::level::debug);//设置输出日志的级别
    if (Debug_Info)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    if (Info_Record)
    {
    }

    // 创建一个对象，传入config
    spdlog::info("Init Server...");
    ntrip_caster a(cfg);
    spdlog::info("Start Server...");
    a.start();

    return 0;
}