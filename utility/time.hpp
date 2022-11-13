#pragma once

#include <chrono>
#include <utility/string.hpp>
#include <utility/platform.hpp>
#ifdef DEVKITPRO
    #include <coreinit/time.h>
#endif

template<typename Clock, Utility::Str::StringLiteral Message = "Process took ">
class Timer {
public:
    void start() {
        begin = Clock::now();
    }

    void stop() {
        end = Clock::now();
        duration = end - begin;
    }

    void print() {
        const std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        Utility::platformLog(std::string(msg) + std::to_string(seconds.count()) + " seconds\n");
    }

private:
    typename Clock::time_point begin;
    typename Clock::time_point end;
    typename Clock::duration duration;

    static constexpr std::string_view msg = Message;
};

template<typename Clock, Utility::Str::StringLiteral Message = "Process took ">
class ScopedTimer : private Timer<Clock, Message> {
public:
    ScopedTimer() {
        Timer<Clock, Message>::start();
    }

    ~ScopedTimer() {
        Timer<Clock, Message>::stop();
        Timer<Clock, Message>::print();
    }
};

//TODO: are these thread safe?
class ProgramTime {
private:
#ifdef DEVKITPRO
    const OSTime openTime;
    static OSTime getOpenedTime();
    static OSTime getElapsedTime();
#else
    using TimePoint_t = std::chrono::system_clock::time_point;

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
