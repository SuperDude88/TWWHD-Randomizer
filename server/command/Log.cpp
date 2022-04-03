#include "Log.hpp"



BasicLog::BasicLog() {}

BasicLog::~BasicLog() {}

BasicLog& BasicLog::getInstance() {
    static BasicLog s_Instance;
    return s_Instance;
}

bool BasicLog::initLog(const std::string &seed) {
    if(!output.is_open()) {
        output.open("TWWHDR-" + seed + " (Non-Spoiler Log).txt");
        return output.is_open();
    }

    return true;
}

void BasicLog::log(const std::string& msg) {
    output << msg << std::endl;
}



DebugLog::DebugLog() {}

DebugLog::~DebugLog() {}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

bool DebugLog::initLog(const std::string &seed) {
    if(!output.is_open()) {
        output.open("TWWHDR-" + seed + " (Debug Log).txt");
        return output.is_open();
    }

    return true;
}

void DebugLog::log(const std::string& msg) {
    output << msg << std::endl;
}
