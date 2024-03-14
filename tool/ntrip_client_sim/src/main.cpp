#include <iostream>
#include <fstream>
#include "ntrip_client.h"
#include "event2/event.h"
#include <nlohmann/json.hpp>
#include "knt/knt.h"

using json = nlohmann::json;

static void Output_Connect_Num(evutil_socket_t fd, short events, void *arg)
{
    int *num = static_cast<int *>(arg);
    std::cout << "Connect Num:" << num << std::endl;
}

int main()
{

    std::ifstream f("ntrip_client_sim_conf.json");

    if (!f.is_open())
    {
        return 1;
    }

    json list = json::parse(f);

    std::string addr = list["Caster_IP"];
    std::string port = list["Caster_Port"];
    int con_num = list["Eatch_Mount_Connect_Num"];

    bool ntrip2 = list["Ntrip_Version"] == 2 ? true : false;
    bool autoNO = list["Auto_Add_NO"] == 0 ? false : true;

    event_base *base = event_base_new();

    for (int i = 0; i < con_num; i++)
    {
        for (auto iter : list["Connection_Mount"])
        {
            ntrip_client *client = new ntrip_client(base);
            std::string mountname;
            if (autoNO)
            {
                mountname = iter;
                mountname += std::to_string(i);
            }
            else
            {
                mountname = iter;
            }

            client->connect(mountname, util_random_string(8) + ':' + util_random_string(8), addr, port, ntrip2);
        }
    }

    // event *output_state = event_new(base, -1, EV_PERSIST, Output_Connect_Num, &connect_num);

    // timeval tv = {5, 0};

    // event_add(output_state, &tv);

    event_base_dispatch(base);

    return 0;
}