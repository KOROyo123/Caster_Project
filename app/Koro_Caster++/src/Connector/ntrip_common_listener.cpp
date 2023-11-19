#include "ntrip_common_listener.h"










ntrip_common_listener::ntrip_common_listener(event_base *base)
{
    _listener = evhttp_new(base);

}

ntrip_common_listener::~ntrip_common_listener()
{

    evhttp_free(_listener);
}
