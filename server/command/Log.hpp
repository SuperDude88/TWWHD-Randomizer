#pragma once

#include <ratio>
#include <string>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "../../seedgen/config.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILENAME__ (&__FILE__[SOURCE_PATH_SIZE])



//make these thread safe?

class ProgramTime {
public: 
    using TimePoint_t = std::chrono::time_point<std::chrono::system_clock>;

    ProgramTime(const ProgramTime&) = delete;
    ProgramTime& operator=(const ProgramTime&) = delete;
private:
    const TimePoint_t openTime;

    ProgramTime();
    ~ProgramTime();

    static ProgramTime& getInstance();

public:
    static TimePoint_t::duration getElapsedTime() { return std::chrono::system_clock::now() - getOpenedTime(); }
    static TimePoint_t getOpenedTime() { return getInstance().openTime; }
};

inline std::string formatTime(const ProgramTime::TimePoint_t::duration& duration_) {
    using namespace std::chrono;
    ProgramTime::TimePoint_t::duration duration = duration_;
    std::stringstream ret;
    ret <<  std::setfill('0');

    hours hr = duration_cast<hours>(duration);
    ret << std::setw(2) << hr.count() << ":";
    duration -= hr;
    minutes min = duration_cast<minutes>(duration);
    ret << std::setw(2) << min.count() << ":";
    duration -= min;
    seconds sec = duration_cast<seconds>(duration);
    ret << std::setw(2) << sec.count() << ".";
    duration -= sec;
    milliseconds ms = duration_cast<milliseconds>(duration);
    ret << std::setw(3) << ms.count();

    return ret.str();
}

class LogInfo {
private:
    Config config;
    std::string seedHash;

    LogInfo();
    ~LogInfo();

    static LogInfo& getInstance();
public:
    LogInfo(const LogInfo&) = delete;
    LogInfo& operator=(const LogInfo&) = delete;
    
    static void setConfig(const Config& config_) { getInstance().config = config_; }
    static void setSeedHash(const std::string& seedHash_) { getInstance().seedHash = seedHash_; }
    static const Config& getConfig();
    static const std::string& getSeedHash();
};

class BasicLog {
private:
    std::ofstream output;

    BasicLog();
    ~BasicLog();
public:
    BasicLog(const BasicLog&) = delete;
    BasicLog& operator=(const BasicLog&) = delete;

    static BasicLog& getInstance();
    void log(const std::string& msg = "");
};

class ErrorLog {
private:
    std::ofstream output;

    ErrorLog();
    ~ErrorLog();
public:
    ErrorLog(const ErrorLog&) = delete;
    ErrorLog& operator=(const ErrorLog&) = delete;

    static ErrorLog& getInstance();
    void log(const std::string& msg = "");
};

#define LOG_ERR_AND_RETURN(error) { \
    ErrorLog::getInstance().log(std::string("Encountered " #error " on line " TOSTRING(__LINE__) " of ") + __FILENAME__); \
    return error; \
}

#define LOG_AND_RETURN_IF_ERR(func) { \
    if(const auto error = func; error != decltype(error)::NONE) {\
        ErrorLog::getInstance().log(std::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__); \
        return error;  \
    } \
}



class DebugLog {
private:
    std::ofstream output;

    DebugLog();
    ~DebugLog();
public:
    DebugLog(const DebugLog&) = delete;
    DebugLog& operator=(const DebugLog&) = delete;

    static DebugLog& getInstance();
    void log(const std::string& msg = "");
};

#ifdef ENABLE_DEBUG
#define LOG_TO_DEBUG(message) \
    DebugLog::getInstance().log(std::string("Message on line " TOSTRING(__LINE__) " of ") + __FILENAME__ + std::string(": " + std::string(message)));
#else
#define LOG_TO_DEBUG(message)
#endif
