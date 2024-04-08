#pragma once
// 记录方式：
// 路径和文件命名方式：
// 基站：Path/Base/2024.04.07/挂载点/Connect_Key.log
// 用户：Path/Rover/2024.04.07/用户名/Connect_Key.log


#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"


std::shared_ptr<spdlog::logger> FLog;


namespace file_loger
{
    int init_file_loger(std::string path);

    int base_write_file();
    int rover_write_file();

}



