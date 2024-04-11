#include "redis_info_push.h"

#include <iostream>

int main()
{

    redis_info_push svr;

    svr.Init(nullptr);

    svr.Start();

    server_info_item base;

    client_info_item rover;

    base.mount_point = "KORO991";
    base.data_type = "RTCM";
    base.blh[0] = 22.260058838767854;
    base.blh[1] = 110.52950882121580;
    base.blh[2] = 134.41942622512579;
    base.rover_num = 1;
    base.online_time = "2024-04-10 10:39:08";
    base.online_seconds = 3721;
    base.ip = "127.0.0.1";
    base.port = 32287;
    base.receive_speed = 621;
    base.receive_total = 3655651;

    rover.user_name = "test0410-1";
    rover.mount_point = "KORO991";
    rover.use_station = "KORO991";
    rover.blh[0] = 32.369135458666662;
    rover.blh[1] = 119.43459318650000;
    rover.blh[2] = 11.970499999999999;

    rover.speed = 0.0000;
    rover.gpsStatus = 1;
    rover.usedSatNum = 37;
    rover.diffTime = 1.000;
    rover.distance = 1286250.0555367079;
    rover.base_blh[0] = 22.260058838767854;
    rover.base_blh[1] = 110.52950882121580;
    rover.base_blh[2] = 134.41942622512579;

    rover.gpsTime_year = 2024;
    rover.gpsTime_mouth = 4;
    rover.gpsTime_day = 10;
    rover.gpsTime_hour = 2;
    rover.gpsTime_min = 52;
    rover.gpsTime_sec = 21.71;

    rover.online_time = "2024-04-10 10:39:08";
    rover.online_seconds = 46;

    rover.ip = "127.0.0.1";
    rover.port = 32287;
    rover.send_speed = 621;
    rover.send_total = 135441;

    auto basevalue = svr.parse_base_to_value(base);

    auto rovervalue = svr.parse_rover_to_value(rover);

    std::cout << basevalue << std::endl;

    std::cout << rovervalue << std::endl;
    return 0;
}