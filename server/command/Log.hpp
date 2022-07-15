#pragma once

#include <string>
#include <fstream>
#include <typeinfo>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILENAME__ (&__FILE__[SOURCE_PATH_SIZE])



class BasicLog {
private:
    std::ofstream output;

    BasicLog();
    ~BasicLog();
public:
    BasicLog(const BasicLog&) = delete;
    BasicLog& operator=(const BasicLog&) = delete;

    static BasicLog& getInstance();
    void logBasicInfo(const std::string& seed);
    void log(const std::string& msg = "");
};

#define LOG_ERR_AND_RETURN(error) {\
    BasicLog::getInstance().log(std::string("Encountered " #error " on line " TOSTRING(__LINE__) " of ") + __FILENAME__); \
    return error; \
}

#define LOG_AND_RETURN_IF_ERR(func) { /*TODO: maybe make version that returns bool*/ \
    if(const auto error = func; error != decltype(error)::NONE) {\
        BasicLog::getInstance().log(std::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__); \
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
    void logBasicInfo(const std::string& seed);
    void log(const std::string& msg = "");
};

#define LOG_TO_DEBUG(message) \
    DebugLog::getInstance().log(std::string("Message on line " TOSTRING(__LINE__) " of ") + __FILENAME__ + std::string(": \n    " + message));
