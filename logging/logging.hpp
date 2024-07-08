#pragma once

#include <cstdarg>

void logit(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#define INFO(fmt, ...) logit(MODULE_NAME "_INFO: " fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) logit(MODULE_NAME "_ERROR: " fmt, ##__VA_ARGS__)