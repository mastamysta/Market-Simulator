#pragma once

#include <cstdio>
#include <cstdarg>

void logit(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);
}

#ifndef LOG_LEVEL_INFO
    #define INFO(fmt, ...)
#else
    #define INFO(fmt, ...) logit(MODULE_NAME "_INFO: " fmt, ##__VA_ARGS__)
#endif 

#define ERROR(fmt, ...) logit(MODULE_NAME "_ERROR: " fmt, ##__VA_ARGS__)