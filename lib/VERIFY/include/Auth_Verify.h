#pragma once
#include <event2/event.h>

typedef void (*VerifyCallback)(const char *request, void *arg, const char *data, size_t data_length);

namespace AUTH
{

    int Init(const char *json_conf, event_base *base);
    int Free();

    int Clear();

} // namespace CASTER

// Redis 内部维护表
// 一张
