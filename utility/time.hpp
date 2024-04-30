#pragma once

#include <chrono>
#include <command/Log.hpp>
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
        LOG_TO_DEBUG(std::string(msg) + (msg.back() == ' ' ? "" : " ") + std::to_string(seconds.count()) + " seconds\n");
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
//gcc __cpp_lib_chrono is 201611L despite having time zones? (https://en.cppreference.com/w/cpp/feature_test#Library_features)
//just check for <format> since that's the only error I've been getting
#if __has_include(<format>) && !defined(__APPLE__)
    using TimePoint_t = std::chrono::zoned_time<std::chrono::system_clock::duration>;
    #define USE_CXX_20_TIME
#else
    using TimePoint_t = std::chrono::system_clock::time_point;
#endif

    const TimePoint_t openTime;
    static TimePoint_t getOpenedTime();
    static TimePoint_t::duration getElapsedTime();

    ProgramTime();
    ~ProgramTime() = default;

public:
    ProgramTime(const ProgramTime&) = delete;
    ProgramTime& operator=(const ProgramTime&) = delete;

    static ProgramTime& getInstance();
    static std::string getTimeStr();
    static std::string getDateStr();
};
