#pragma once

#include <string>
#include <fstream>
#include <typeinfo>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <array>
#include <list>
#ifdef DEVKITPRO
    #include <coreinit/time.h>
#endif

#include <seedgen/config.hpp>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILENAME__ (&__FILE__[SOURCE_PATH_SIZE])

//TODO: are these thread safe?
class ProgramTime {
private:
#ifdef DEVKITPRO
    const OSTime openTime;
    static OSTime getOpenedTime();
    static OSTime getElapsedTime();
#else
    using TimePoint_t = std::chrono::time_point<std::chrono::system_clock>;

    const TimePoint_t openTime;
    static TimePoint_t getOpenedTime();
    static TimePoint_t::duration getElapsedTime();
#endif

    ProgramTime();
    ~ProgramTime();
public:
    ProgramTime(const ProgramTime&) = delete;
    ProgramTime& operator=(const ProgramTime&) = delete;

    static ProgramTime& getInstance();
    static std::string getTimeStr();
    static std::string getDateStr();
};

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
    void close();
};

class ErrorLog {
private:
    static constexpr size_t MAX_ERRORS = 5;

    std::ofstream output;
    std::list<std::string> lastErrors;

    ErrorLog();
    ~ErrorLog();
public:
    ErrorLog(const ErrorLog&) = delete;
    ErrorLog& operator=(const ErrorLog&) = delete;

    static ErrorLog& getInstance();
    void log(const std::string& msg = "");
    std::string getLastErrors() const;
    void clearLastErrors();
    void close();
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
    void close();
};

#ifdef ENABLE_DEBUG
#define LOG_TO_DEBUG(message) \
    DebugLog::getInstance().log(std::string("Message on line " TOSTRING(__LINE__) " of ") + __FILENAME__ + std::string(": " + std::string(message)));
#else
#define LOG_TO_DEBUG(message)
#endif
