#include "Log.hpp"

#include <version.hpp>



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
    output.open(APP_SAVE_PATH + LogInfo::getSeedHash() + " Non-Spoiler Log.txt");

    output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << LogInfo::getConfig().seed << std::endl;

    output << "Selected options:" << std::endl << "\t";
    for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
    {
        Option setting = static_cast<Option>(settingInt);

        if (setting == Option::NumShards || setting == Option::NumRequiredDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
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

void BasicLog::log(const std::string& msg, const bool& timestamp) {
    if(timestamp) output << "[" << ProgramTime::getTimeStr() << "] ";
    output << msg << std::endl;
}

void BasicLog::close()
{
    output.close();
}



ErrorLog::ErrorLog() {
    output.open(APP_SAVE_PATH "Error Log.txt");

    output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;

    // Only list config stuff if a config has been set
    if (LogInfo::getConfig().configSet)
    {
        output << "Seed: " << LogInfo::getConfig().seed << std::endl;
        output << "Selected options:" << std::endl << "\t";
        for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
        {
            Option setting = static_cast<Option>(settingInt);
        
            if (setting == Option::NumShards || setting == Option::NumRequiredDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
            {
                output << settingToName(setting) << ": " << std::to_string(getSetting(LogInfo::getConfig().settings, setting)) << ", ";
            }
            else
            {
                output << (getSetting(LogInfo::getConfig().settings, setting) ? settingToName(setting) + ", " : "");
            }
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

void ErrorLog::log(const std::string& msg, const bool& timestamp) {
    if(timestamp) output << "[" << ProgramTime::getTimeStr() << "] ";
    output << msg << std::endl;
    lastErrors.push_back(msg);
    if (lastErrors.size() > MAX_ERRORS)
    {
        lastErrors.pop_front();
    }
}

std::string ErrorLog::getLastErrors() const
{
    std::string retStr = "";
    for (auto& error : lastErrors)
    {
        retStr += error + "\n";
    }
    return retStr;
}

void ErrorLog::clearLastErrors()
{
    lastErrors.clear();
}

void ErrorLog::close()
{
    output.close();
}



DebugLog::DebugLog() {
    #ifdef ENABLE_DEBUG
        output.open(APP_SAVE_PATH "Debug Log.txt");

        output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

        output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
        // output << "Seed: " << LogInfo::getConfig().seed << std::endl;
        //
        // output << "Selected options:" << std::endl << "\t";
        // for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++)
        // {
        //     Option setting = static_cast<Option>(settingInt);
        //
        //     if (setting == Option::NumShards || setting == Option::NumRequiredDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor)
        //     {
        //         output << settingToName(setting) << ": " << std::to_string(getSetting(LogInfo::getConfig().settings, setting)) << ", ";
        //     }
        //     else
        //     {
        //         output << (getSetting(LogInfo::getConfig().settings, setting) ? settingToName(setting) + ", " : "");
        //     }
        // }

        output << std::endl << std::endl;
    #endif
}

DebugLog::~DebugLog() {

}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

void DebugLog::log(const std::string& msg, const bool& timestamp) {
    #ifdef ENABLE_DEBUG
        if(timestamp) output << "[" << ProgramTime::getTimeStr() << "] ";
        output << msg << std::endl;
    #endif
}

void DebugLog::close()
{
    output.close();
}
