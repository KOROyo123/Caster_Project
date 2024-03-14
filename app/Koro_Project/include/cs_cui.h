#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "cs_core.h"

class cs_cui
{
private:
    /* data */

    json _conf;

    //数据处理模块
    cs_core _core;



public:
    cs_cui(/* args */);
    ~cs_cui();

    void server_loop();


    int load_project(json conf);
    int save_project(std::string save_path);
    int clear_project();

    json get_setting();
    int set_setting(json item);

    int load_base(json base_item);
    int load_rover(json rover_item);
    int load_nav(json nav_item);


};

