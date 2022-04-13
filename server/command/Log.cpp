#include "Log.hpp"



BasicLog::BasicLog() {
    output.open("Non-Spoiler Log.txt");
}

BasicLog::~BasicLog() {}

BasicLog& BasicLog::getInstance() {
    static BasicLog s_Instance;
    return s_Instance;
}

void BasicLog::logBasicInfo(const std::string &seed) {
    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << seed << std::endl;
}

void BasicLog::log(const std::string& msg) {
    output << msg << std::endl;
}



DebugLog::DebugLog() {
    output.open("Debug Log.txt");
}

DebugLog::~DebugLog() {}

DebugLog& DebugLog::getInstance() {
    static DebugLog s_Instance;
    return s_Instance;
}

void DebugLog::logBasicInfo(const std::string &seed) {
    output << "Wind Waker HD Randomizer Version " << RANDOMIZER_VERSION << std::endl;
    output << "Seed: " << seed << std::endl;
}

void DebugLog::log(const std::string& msg) {
    output << msg << std::endl;
}
