/*
    https://gpsd.io/NMEA.html

*/
#pragma once

#include <memory>


namespace NMEA
{

    struct GGA
    {
        char talker_ID[16];
        /* data */
    };

    struct GSA
    {
        char talker_ID[16];
        /* data */
    };

    struct GSV
    {
        char talker_ID[16];
        /* data */
    };

    struct RMC
    {
        char talker_ID[16];
        /* data */
    };

    enum nmea_type
    {
        TYPE_GGA,
        TYPE_GSA,
        TYPE_GSV,
        TYPE_RMC
    };

    struct Nmea_t
    {
        nmea_type type;

        char talker_ID[16];

        /* data */
    };

    Nmea_t *Auto_Decode(char * message);

    GGA* Decode_GGA(char * message);

    GSA* Decode_GSA(char * message);

    GSV* Decode_GSV(char * message);

    RMC* Decode_RMC(char * message);

    //void Free_Decode(auto *decode_nmea);

}
