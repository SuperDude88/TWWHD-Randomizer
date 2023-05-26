#include "time.hpp"

#include <format>

using namespace std::chrono;
using namespace std::literals::chrono_literals;



ProgramTime::ProgramTime() :
    openTime(current_zone(), system_clock::now())
{}

ProgramTime::TimePoint_t ProgramTime::getOpenedTime() {
    return getInstance().openTime;
}

ProgramTime::TimePoint_t::duration ProgramTime::getElapsedTime() {
    return system_clock::now() - getOpenedTime().get_sys_time();
}

std::string ProgramTime::getDateStr() {
    const auto& opened = getOpenedTime();
    return std::format("{0:%a %b %d %Y %I:%M:%S %p} {1:%Z%n}", round<seconds>(opened.get_local_time()), opened); //TODO: fix wii u timezone being wrong
}

std::string ProgramTime::getTimeStr() {
    return std::format("{:%T}", round<milliseconds>(getElapsedTime()));
}

ProgramTime::~ProgramTime() {}

ProgramTime& ProgramTime::getInstance() {
    static ProgramTime s_Instance;
    return s_Instance;
}

static const ProgramTime& temp = ProgramTime::getInstance(); //inaccessible global to create instance when program starts
