#include "Log.hpp"

#include <version.hpp>
#include <utility/time.hpp>



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



static void printBasicInfo(std::ostream& output) {
    output << "Program opened " << ProgramTime::getDateStr(); // time string ends with \n

    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;

    // Only list config stuff if a config has been set
    if (LogInfo::getConfig().configSet) {
        output << "Seed: " << LogInfo::getConfig().seed << std::endl;
        output << "Selected options:" << std::endl << "\t";
        for (int settingInt = 1; settingInt < static_cast<int>(Option::COUNT); settingInt++) {
            const Option setting = static_cast<Option>(settingInt);
        
            if (setting == Option::NumRequiredDungeons || setting == Option::DamageMultiplier || setting == Option::PigColor) {
                output << settingToName(setting) << ": " << std::to_string(LogInfo::getConfig().settings.getSetting(setting)) << ", ";
            }
            else if(LogInfo::getConfig().settings.getSetting(setting)) {
                output << settingToName(setting) + ", ";
            }
        }

        output << std::endl;
    }

    output << std::endl;
}



ErrorLog::ErrorLog() {
    output.open(LOG_PATH);

    printBasicInfo(output);
}

ErrorLog::~ErrorLog() {
    output.close();
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



DebugLog::DebugLog() {
    output.open(LOG_PATH);

    printBasicInfo(output);
}

DebugLog::~DebugLog() {
    output.close();
}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

void DebugLog::log(const std::string& msg, const bool& timestamp) {
    if(timestamp) output << "[" << ProgramTime::getTimeStr() << "] ";
    output << msg << std::endl;
}
