#include "time.hpp"


using namespace std::chrono;
using namespace std::literals::chrono_literals;



ProgramTime::ProgramTime() :
    openTime(system_clock::now())
{}

ProgramTime& ProgramTime::getInstance() {
    static ProgramTime s_Instance;
    return s_Instance;
}

ProgramTime::TimePoint_t ProgramTime::getOpenedTime() {
    return getInstance().openTime;
}

#ifdef USE_CXX_20_TIME
    #include <format>

    ProgramTime::TimePoint_t::duration ProgramTime::getElapsedTime() {
        return system_clock::now() - getOpenedTime().get_sys_time();
    }

    std::string ProgramTime::getDateStr() {
        const zoned_time opened(current_zone(), getOpenedTime());
        return std::format("{0:%a %b %d %Y %I:%M:%S %p} {1:%Z%n}", round<seconds>(opened.get_local_time()), opened); //TODO: fix wii u timezone being wrong
    }

    std::string ProgramTime::getTimeStr() {
        return std::format("{:%T}", round<milliseconds>(getElapsedTime()));
    }
#else
    #include <sstream>
    #include <mutex>

    ProgramTime::TimePoint_t::duration ProgramTime::getElapsedTime() {
        return system_clock::now() - getOpenedTime();
    }

    std::string ProgramTime::getDateStr() {
        const time_t point = system_clock::to_time_t(ProgramTime::getOpenedTime());

        static std::mutex localtimeMut; //std::ctime is not thread safe
        std::unique_lock<std::mutex> lock(localtimeMut);
        return std::ctime(&point); //time string ends with \n
    }

    std::string ProgramTime::getTimeStr() {
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

static const ProgramTime& temp = ProgramTime::getInstance(); //inaccessible global to create instance when program starts
