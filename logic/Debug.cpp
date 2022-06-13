
#include "Debug.hpp"

#include <iostream>
#include <fstream>

static std::ofstream logger;

void debugLog(const std::string& msg /*= ""*/)
{
    #ifdef ENABLE_DEBUG
        logger << msg << std::endl;
    #endif
}

void openDebugLog(const std::string& seed)
{
    #ifdef ENABLE_DEBUG
        logger.open("debug_logger" + seed + ".txt");
    #endif
}

void closeDebugLog()
{
    #ifdef ENABLE_DEBUG
        logger.close();
    #endif
}
