#include <iostream>

#include "nlohmann/json.hpp"
#include <memory>

using json = nlohmann::json;

int main()
{

    //初始化日志系统

    
    //读取配置文件


    //读取全局配置
    std::shared_ptr<json> global_config(new json);


    //配置文件内容检查，避免程序挂掉
    

    //启动一个对象，传入全局配置






    return 0;
}