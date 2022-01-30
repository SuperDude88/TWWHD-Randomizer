
#include "Debug.hpp"

#include <iostream>
#include <fstream>

static std::ofstream log;

void debugLog(const std::string& msg)
{
    #ifdef ENABLE_DEBUG
        log << msg << std::endl;
    #endif
}

void openDebugLog()
{
    #ifdef ENABLE_DEBUG
        log.open("debug_log.txt");
    #endif
}

void closeDebugLog()
{
    #ifdef ENABLE_DEBUG
        log.close();
    #endif
}
