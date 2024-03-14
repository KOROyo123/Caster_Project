#include "file_loger.h"




int file_loger::init_file_loger(std::string path)
{
    auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
    FLog = logger;

    return 0;

}
