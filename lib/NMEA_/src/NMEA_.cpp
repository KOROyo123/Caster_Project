#include "NMEA_.h"

namespace NMEA
{

    Nmea_t *Auto_Decode(char *message)
    {
        return nullptr;
    }

    GGA *Decode_GGA(char *message)
    {
        return nullptr;
    }
    GSA *Decode_GSA(char *message)
    {
        return nullptr;
    }
    GSV *Decode_GSV(char *message)
    {
        return nullptr;
    }
    RMC *Decode_RMC(char *message)
    {
        return nullptr;
    }
    void Free_Decode(auto *decode_nmea)
    {


       free(decode_nmea);
       
    }
}