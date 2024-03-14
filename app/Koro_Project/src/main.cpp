#include "cs_cui.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main()
{
    // 创建一个实例对象
    cs_cui a;

    // 监听键盘输入
    // 根据输入执行对应的操作
    a.server_loop();

    // 具体功能设计---------------------------------------------------------------------------

    //-help 帮助
    //-version 版本信息

    //-new 新建项目（打开模板json文件，赋值给pro，修改指定参数）
    json pro1;
    a.load_project(pro1);

    //-open 打开历史项目（打开本地的工程文件，赋值给pro）
    json pro2;
    a.load_project(pro2);

    //-setting 查看配置
    a.get_setting();

    //-setting xxxx xxxx 修改配置
    json conf;
    conf["xxxx"] = "xxxx";
    a.set_setting(conf);

    // 添加观测文件
    //-obsb 添加基站文件（文件路径、站点名称、站点坐标、天线参数等）
    json obs1;
    obs1["name"]="base1";
    obs1["lat"]="35.9995";
    obs1["lon"]="120.1121";
    obs1["height"]="50.963";
    obs1["ant_h"]="1.450";
    obs1["file_path"][0]="/data/BASE01_202402181805.24O";
    obs1["file_path"][1]="/data/BASE01_202402190000.24O";
    obs1["file_path"][2]="/data/BASE01_202402200000.24O";
    a.load_base(obs1);

    //-obsr 添加测站文件（文件路径、站点名称、天线参数等）
    json obs2;
    obs2["name"]="rover1";
    obs2["ant_h"]="0.000";
    obs2["file_path"][0]="/data/TEST01_202402181915.24O";
    a.load_rover(obs2);
    //-navb 添加星历文件（文件路径）
    json nav;
    nav["file_path"][0]="/data/BASE01_202402181805.24N";
    nav["file_path"][1]="/data/BASE01_202402190000.24N";
    nav["file_path"][2]="/data/BASE01_202402200000.24N";
    a.load_nav(nav);

    // 获取观测文件
    // 读取基站列表
    // 读取移动站列表
    // 读取星历列表

    // 修改观测文件
    // 更新基站配置参数
    // 更新移动站配置参数
    // 更新星历配置参数

    // 删除观测文件
    // 删除基站
    // 删除移动站
    // 删除星历

    // 数据处理配置
    json proc;

    proc["a"]="";


    // 数据处理
    // 单点定位

    // 动态基线

    return 0;
}