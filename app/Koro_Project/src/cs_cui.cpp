#include "cs_cui.h"



cs_cui::cs_cui(/* args */)
{
}

cs_cui::~cs_cui()
{
}

void cs_cui::server_loop()
{
}

int cs_cui::load_project(json conf)
{
    return 0;
}

int cs_cui::save_project(std::string save_path)
{
    return 0;
}

int cs_cui::clear_project()
{
    return 0;
}

json cs_cui::get_setting()
{
    return json();
}

int cs_cui::set_setting(json)
{
    return 0;
}

int cs_cui::load_base(json base_item)
{
    return 0;
}

int cs_cui::load_rover(json rover_item)
{
    return 0;
}

int cs_cui::load_nav(json nav_item)
{
    return 0;
}
