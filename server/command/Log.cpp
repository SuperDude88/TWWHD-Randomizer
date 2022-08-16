#include "Log.hpp"
#include <chrono>
#include <mutex>



#ifdef DEVKITPRO
    ProgramTime::ProgramTime() :
        openTime(OSGetTime())
    {}

    OSTime ProgramTime::getOpenedTime() {
        return getInstance().openTime;
    }

    OSTime ProgramTime::getElapsedTime() {
        return OSGetTime() - getOpenedTime();
    }

    std::string ProgramTime::getDateStr() {
        //IMPROVEMENT: maybe convert 24 hour time to 12AM/PM?
        static const std::array<std::string, 12> MonthStrings {
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec"
        };
        static const std::array<std::string, 7> DayStrings {
            "Sun",
            "Mon",
            "Tue",
            "Wed",
            "Thu",
            "Fri",
            "Sat"
        };

        OSCalendarTime time;
        OSTicksToCalendarTime(getOpenedTime(), &time);
        std::stringstream ret;

        ret << DayStrings[time.tm_wday] << " ";
        ret << MonthStrings[time.tm_mon] << " ";
        ret << time.tm_mday << " ";
        ret << std::setfill('0') << std::setw(2) << time.tm_hour << ":";
        ret << std::setw(2) << time.tm_min << ":";
        ret << std::setw(2) << time.tm_sec << " ";
        ret << time.tm_year << "\n";

        return ret.str();
    }

    std::string ProgramTime::getTimeStr() {
        const OSTime duration = getElapsedTime();
        OSCalendarTime time;
        OSTicksToCalendarTime(duration, &time);
        std::stringstream ret;
        ret << std::setfill('0');

        ret << std::setw(2) << time.tm_hour << ":";
        ret << std::setw(2) << time.tm_min << ":";
        ret << std::setw(2) << time.tm_sec << ".";
        ret << std::setw(3) << time.tm_msec;

        return ret.str();
    }
#else
    ProgramTime::ProgramTime() :
        openTime(std::chrono::system_clock::now())
    {}

    ProgramTime::TimePoint_t ProgramTime::getOpenedTime() {
        return getInstance().openTime;
    }

    ProgramTime::TimePoint_t::duration ProgramTime::getElapsedTime() {
        return std::chrono::system_clock::now() - getOpenedTime();
    }

    std::string ProgramTime::getDateStr() {
        //IMPROVEMENT: use once c++20 is better supported (gcc/clang)
        //also maybe replace %T with something more specific
        //return std::format("{:%a %b %d %T %Y %n}\n", ProgramTime::getOpenedTime()); 

        const time_t point = std::chrono::system_clock::to_time_t(ProgramTime::getOpenedTime());

        static std::mutex localtimeMut; //std::ctime is not thread safe
        std::unique_lock<std::mutex> lock(localtimeMut);
        return std::ctime(&point); //time string ends with \n
    }

    std::string ProgramTime::getTimeStr() {
        using namespace std::chrono;

        TimePoint_t::duration duration = getElapsedTime();
        std::stringstream ret;
        ret <<  std::setfill('0');

        const hours hr = duration_cast<hours>(duration);
        ret << std::setw(2) << hr.count() << ":";
        duration -= hr;
        const minutes min = duration_cast<minutes>(duration);
        ret << std::setw(2) << min.count() << ":";
        duration -= min;
        const seconds sec = duration_cast<seconds>(duration);
        ret << std::setw(2) << sec.count() << ".";
        duration -= sec;
        const milliseconds ms = duration_cast<milliseconds>(duration);
        ret << std::setw(3) << ms.count();

        return ret.str();
    }
#endif

ProgramTime::~ProgramTime() {}

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

    output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

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
    output << "[" << ProgramTime::getTimeStr() << "] " << msg << std::endl;
}

void BasicLog::close()
{
    output.close();
}



ErrorLog::ErrorLog() {
    output.open("./Error Log.txt");

    output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

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

#include "../utility/platform.hpp"

void ErrorLog::log(const std::string& msg) {
    //TODO: remove print after testing
    // Utility::platformLog(msg + '\n');
    output << "[" << ProgramTime::getTimeStr() << "] " << msg << std::endl;
    lastErrors.push_back(msg);
    if (lastErrors.size() > 5)
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
        output.open("./Debug Log.txt");

        output << "Program opened " << ProgramTime::getDateStr(); //time string ends with \n

        output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
        output << "Seed: " << LogInfo::getConfig().seed << std::endl;

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
    #endif
}

DebugLog::~DebugLog() {

}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

void DebugLog::log(const std::string& msg) {
    #ifdef ENABLE_DEBUG
        output << "[" << ProgramTime::getTimeStr() << "] " << msg << std::endl;
    #endif
}

void DebugLog::close()
{
    output.close();
}
