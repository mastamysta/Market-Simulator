#pragma once


// Just a number of milis from the beginning of the day.
struct time
{
    unsigned long long int milis;
};

enum instrument
{
    EURUSD = 0,
    USDJPY = 1,
    GBPUSD = 2
};

const std::map<std::string, instrument> instrument_map =
{
    { "EURUSD" , EURUSD },
    { "GBPUSD" , GBPUSD },
    { "USDJPY" , USDJPY }
};

struct state
{
    instrument ins;

    double ask;
    double bid;
    double askVol;
    double bidVol;
};

using tick = std::pair<struct time, struct state>;
