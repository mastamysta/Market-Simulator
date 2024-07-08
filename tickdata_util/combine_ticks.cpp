#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <tuple>
#include <sstream>
#include <map>

#include "tick_data.hpp"
#include "logging.hpp"

#define MODULE_NAME "TICK_COMBINER"


tick generate_tick_from_line(instrument instrument, const std::string& line)
{
    struct time t = { 0 };
    struct state s;

    s.ins = instrument;

    // DD.MM.YYYY HH:MM:SS.mmm
    unsigned long long hours = std::stoi(line.substr(11, 2));
    unsigned long long mins = std::stoi(line.substr(14, 2));
    mins += hours * 60;
    unsigned long long secs = std::stoi(line.substr(17, 2));
    secs += mins * 60;
    t.milis = std::stoi(line.substr(20, 3));
    t.milis += secs * 1000;

    // Remainder of line is comma delimited
    std::string token;
    std::istringstream ssn(line);

    // chuck away first token, containing timestamp.
    std::getline(ssn, token, ',');

    std::getline(ssn, token, ',');
    s.ask = std::stod(token);

    std::getline(ssn, token, ',');
    s.bid = std::stod(token);

    std::getline(ssn, token, ',');
    s.askVol = std::stod(token);

    std::getline(ssn, token, ',');
    s.bidVol = std::stod(token);

    return { t, s };
}

instrument get_instrument_from_filename(const char *filename)
{
    std::string fnm = filename;
    std::istringstream ssn(fnm);
    std::string rootname;

    while (std::getline(ssn, rootname, '/')) { ; }

    char instr_buf[8];
    memcpy(instr_buf, rootname.c_str(), 6);
    instr_buf[7] = '\0';

    if (instrument_map.find(instr_buf) == instrument_map.end())
    {
        ERROR("Invalid instrument name found '%s'.\n", instr_buf);
        exit(-1);
    }

    return instrument_map.find(instr_buf)->second;
}

void parse_file(std::vector<tick>& ticklist, const char *filename)
{
    std::ifstream file;
    file.open(filename);

    if (!file.is_open())
    {
        ERROR("Failed to open file: ", filename);
        exit(-1);
    }

    // Chuck away first line containing column labels.
    std::string tmp;
    std::getline(file, tmp);

    auto instrument = get_instrument_from_filename(filename);

    for (std::string line; std::getline(file, line); ) 
    {
        ticklist.push_back(generate_tick_from_line(instrument, line));
    }
}

bool compare(const tick& lhs, const tick& rhs)
{
    return get<0>(lhs).milis < get<0>(rhs).milis;
}

void parse_files(std::vector<tick>& ticklist, int argc, const char *argv[])
{
    // Skip first element, command line
    for (int i = 1; i < argc; i++)
    {
        parse_file(ticklist, argv[i]);
    }

    // Sort elements by milisecond
    std::sort(ticklist.begin(), ticklist.end(), compare);
}

void dump_ticklist(const std::vector<tick>& ticklist)
{
    std::ofstream dumpfile;
    dumpfile.open("COMBINED_TICK.csv");

    if (!dumpfile.is_open())
    {
        ERROR("Failed to create dumpfile for combined data.\n");
    }

    for (auto tick : ticklist)
    {
        dumpfile << tick.second.ins << "," << tick.first.milis << "," << tick.second.ask << "," << tick.second.bid \
                << "," << tick.second.askVol << "," << tick.second.bidVol << "\n"; 
    }
}

int main(int argc, const char *argv[])
{

    if (argc == 1)
        ERROR("No tick data files supplied.\n");

    std::vector<tick> ticklist;

    parse_files(ticklist, argc, argv);

    dump_ticklist(ticklist);

    return 0;

}