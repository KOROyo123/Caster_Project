#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;







class cs_core
{
private:
    //json配置文件
    json _conf;

    //观测文件相关数据
    //站点观测数据
    



public:
    cs_core(/* args */);
    ~cs_core();

    //CS_GUI调用接口
    int setConf(json conf);
    json getConf();

    int startProcess()

    int getResult();
    int getObsInfo();
    int getNavInfo();




};

