#include <iostream>
#include <fstream>

#include "logging.hpp"

#define MODULE_NAME "MARKET_SIM"

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        ERROR("Need to supply a market order data file from LOBSTER.\n");
    }
}