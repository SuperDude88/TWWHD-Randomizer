#pragma once

#include <chrono>
#include <utility/string.hpp>
#include <utility/platform.hpp>

template<typename Clock, Utility::Str::StringLiteral Message = "Process took ">
class Timer {
public:
    typename Clock::duration getElapsed() const {
        end = Clock::now();
        return end - begin;
    }

protected:
    typename Clock::time_point begin;
    typename Clock::time_point end;

    void start() {
        begin = Clock::now();
    }

    void stop() {
        end = Clock::now();
        duration = end - begin;
    }
    
    void print() const {
        const std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        Utility::platformLog(std::string(msg) + (msg.back() == ' ' ? "" : " ") + std::to_string(seconds.count()) + " seconds\n");
    }

private:
    typename Clock::duration duration;

    static constexpr std::string_view msg = Message;
};

template<typename Clock, Utility::Str::StringLiteral Message = "Process took ">
class ScopedTimer : public Timer<Clock, Message> {
public:
    ScopedTimer() {
        Timer<Clock, Message>::start();
    }

    ~ScopedTimer() {
        Timer<Clock, Message>::stop();
        Timer<Clock, Message>::print();
    }
};

class ProgramTime {
private:
    using TimePoint_t = std::chrono::zoned_time<std::chrono::system_clock::duration>;

    const TimePoint_t openTime;
    static TimePoint_t getOpenedTime();
    static TimePoint_t::duration getElapsedTime();

    ProgramTime();
    ~ProgramTime();
public:
    ProgramTime(const ProgramTime&) = delete;
    ProgramTime& operator=(const ProgramTime&) = delete;

    static ProgramTime& getInstance();
    static std::string getTimeStr();
    static std::string getDateStr();
};
