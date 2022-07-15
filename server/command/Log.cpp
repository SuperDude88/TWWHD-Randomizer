#include "Log.hpp"
#include <chrono>
#ifdef DEVKITPRO
    #include <coreinit/time.h>
#endif



ProgramTime::ProgramTime() :
    openTime(std::chrono::system_clock::now())
{

}

ProgramTime::~ProgramTime()
{

}

ProgramTime& ProgramTime::getInstance() {
    static ProgramTime s_Instance;
    return s_Instance;
}



LogInfo::LogInfo()
{

}

LogInfo::~LogInfo() {

}

LogInfo& LogInfo::getInstance() {
    static LogInfo s_Instance;
    return s_Instance;
}

const Config& LogInfo::getConfig() {
    return getInstance().config;
}

const std::string& LogInfo::getSeedHash() {
    return getInstance().seedHash;
}



BasicLog::BasicLog() {
    output.open("./Non-Spoiler Log.txt");

    time_t point = std::chrono::system_clock::to_time_t(ProgramTime::getOpenedTime());
    output << "Program opened " << std::ctime(&point); //time string ends with \n

    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << LogInfo::getConfig().seed << std::endl;

    output << "Selected options:" << std::endl << "\t";
    for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
    {
        Option setting = static_cast<Option>(settingInt);

        if (setting == Option::NumShards || setting == Option::NumRaceModeDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
        {
            output << settingToName(setting) << ": " << std::to_string(getSetting(LogInfo::getConfig().settings, setting)) << ", ";
        }
        else
        {
            output << (getSetting(LogInfo::getConfig().settings, setting) ? settingToName(setting) + ", " : "");
        }
    }
    
    output << std::endl << std::endl;
}

BasicLog::~BasicLog() {

}

BasicLog& BasicLog::getInstance() {
    static BasicLog s_Instance;
    return s_Instance;
}

void BasicLog::log(const std::string& msg) {
    output << "[" << formatTime(ProgramTime::getElapsedTime()) << "] " << msg << std::endl;
}



ErrorLog::ErrorLog() {
    output.open("./Error Log.txt");

    time_t point = std::chrono::system_clock::to_time_t(ProgramTime::getOpenedTime());
    output << "Program opened " << std::ctime(&point); //time string ends with \n
    
    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << LogInfo::getConfig().seed << std::endl;

    output << "Selected options:" << std::endl << "\t";
    for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
    {
        Option setting = static_cast<Option>(settingInt);

        if (setting == Option::NumShards || setting == Option::NumRaceModeDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
        {
            output << settingToName(setting) << ": " << std::to_string(getSetting(LogInfo::getConfig().settings, setting)) << ", ";
        }
        else
        {
            output << (getSetting(LogInfo::getConfig().settings, setting) ? settingToName(setting) + ", " : "");
        }
    }

    output << std::endl << std::endl;
}

ErrorLog::~ErrorLog() {
    
}

ErrorLog& ErrorLog::getInstance() {
    static ErrorLog s_Instance;
    return s_Instance;
}

void ErrorLog::log(const std::string& msg) {
    output << "[" << formatTime(ProgramTime::getElapsedTime()) << "] " << msg << std::endl;
}



DebugLog::DebugLog() {
    output.open("./Debug Log.txt");

    time_t point = std::chrono::system_clock::to_time_t(ProgramTime::getOpenedTime());
    output << "Program opened " << std::ctime(&point); //time string ends with \n
    
    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << LogInfo::getConfig().seed << std::endl;

    output << "Selected options:" << std::endl << "\t";
    for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
    {
        Option setting = static_cast<Option>(settingInt);

        if (setting == Option::NumShards || setting == Option::NumRaceModeDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
        {
            output << settingToName(setting) << ": " << std::to_string(getSetting(LogInfo::getConfig().settings, setting)) << ", ";
        }
        else
        {
            output << (getSetting(LogInfo::getConfig().settings, setting) ? settingToName(setting) + ", " : "");
        }
    }

    output << std::endl << std::endl;
}

DebugLog::~DebugLog() {

}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

void DebugLog::log(const std::string& msg) {
    output << "[" << formatTime(ProgramTime::getElapsedTime()) << "] " << msg << std::endl;
}
