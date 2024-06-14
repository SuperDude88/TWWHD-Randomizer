#include "time.hpp"

using namespace std::chrono;
using namespace std::literals::chrono_literals;

ProgramTime::ProgramTime() :
    openTime(Clock_t::now())
{}

ProgramTime& ProgramTime::getInstance() {
    static ProgramTime s_Instance;
    return s_Instance;
}

ProgramTime::TimePoint_t ProgramTime::getOpenedTime() {
    return getInstance().openTime;
}

ProgramTime::Duration_t ProgramTime::getElapsedTime() {
    return Clock_t::now() - getOpenedTime();
}

#if __has_include(<format>) && !defined(__APPLE__)
    #include <format>

    std::string ProgramTime::getDateStr() {
        return std::format("{0:%a, %b %d, %Y, %I:%M:%S %p%n}", round<seconds>(getOpenedTime()));
    }

    std::string ProgramTime::getTimeStr() {
        return std::format("{:%T}", round<milliseconds>(getElapsedTime()));
    }
#else
    #include <sstream>
    #include <mutex>

    std::string ProgramTime::getDateStr() {
        const time_t point = Clock_t::to_time_t(ProgramTime::getOpenedTime());

        static std::mutex localtimeMut; //std::ctime is not thread safe
        std::unique_lock<std::mutex> lock(localtimeMut);
        return std::ctime(&point); //time string ends with \n
    }

    std::string ProgramTime::getTimeStr() {
        Duration_t duration = getElapsedTime();
        std::stringstream ret;
        ret << std::setfill('0');

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

static const ProgramTime& temp = ProgramTime::getInstance(); //inaccessible global to create instance when program starts
