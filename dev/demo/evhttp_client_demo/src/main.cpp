/*

*/

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"

#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include <unistd.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rtklib.h"


#include "http_heart_beat.h"

using json = nlohmann::json;

#define CONF_PATH "/Code/Koro_Caster/app/evhttp_client_demo/heatbeat.json"

// POST /DiffBase/moduleInfo/cdcPlus HTTP/1.1
// Content-Type: application/json
// Host: 127.0.0.1:4202
// Content-Length: 135

// {"PID":13996,"gpssecond":64534,"module":"CDC_Plus","node":"192.168.2.167","onlineServer":1234,"onlineTime":1386400,"onlineclient":2345}





int main()
{

    event_base *base=event_base_new();





    // 创建一个对象
    auto beat = new http_heartbeat();
    // 初始化配置//读取IP,PID，启动时间
    beat->set_url("http://140.207.166.210:9030/DiffBase/moduleInfo/cdcPlus");
    //http://cloud.sinognss.com/gateway/DiffBase/moduleInfo/cdcPlus
    // beat->set_url("http://101.34.217.6:11001/DiffBase/moduleInfo/cdcPlus");
    beat->set_event_base(base);


    sleep(1);
    // for (int i = 0; i < 100; i++)
    // {
        // 获取基站和移动站数量

        // 更新数据
        beat->update_info(100, 99);
        // 发送心跳
        beat->send_heartbeat();
    // }


    event_base_dispatch(base);

}
