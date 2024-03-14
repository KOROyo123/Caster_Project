#pragma once


#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"


std::shared_ptr<spdlog::logger> FLog;


namespace file_loger
{
    int init_file_loger(std::string path);
}



