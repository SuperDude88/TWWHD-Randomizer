#pragma once

#include <chrono>

#include <vpad/input.h>

enum struct [[nodiscard]] InputError {
    NONE = 0,
    NO_SAMPLES,
    INVALID_CONTROLLER,
    BUSY,
    UNINITIALIZED,
    UNKNOWN,
    COUNT
};

class InputManager {
private:
    InputManager() {}
    ~InputManager() {}

public:
    using Clock_t = std::chrono::high_resolution_clock;
    using Duration_t = Clock_t::duration;
    using Time_t = Clock_t::time_point;

    class RepeatInfo {
    private:
        Duration_t delay;
        Duration_t interval;

        bool wait;
        Time_t last_pressed;

    public:
        RepeatInfo(const Duration_t& delay_ = Duration_t(-1), const Duration_t& interval_ = Duration_t(-1)) :
            delay(delay_),
            interval(interval_),
            wait(true)
        {}

        bool doRepeat() {
            return delay > Duration_t(0) || interval > Duration_t(0);
        }

        bool doWait() { return wait; }
        void setWait(const bool& wait_) { wait = wait_; }
        Duration_t getDelay() { return delay; }
        Duration_t getInterval() { return interval; }
        Time_t getLastPressed() { return last_pressed; }
        void setLastPressed(const Time_t& time) { last_pressed = time; }
    };

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    static InputManager& getInstance();

    InputError poll();
    bool pressed(const VPADButtons& buttons);
    bool held(const VPADButtons& buttons);
    bool released(const VPADButtons& buttons);
    void setRepeat(const VPADButtons& buttons, const Duration_t& delay, const Duration_t& interval);
    void clearRepeat(const VPADButtons& buttons);

private:
    using Buttons_t = std::underlying_type_t<VPADButtons>;
    Buttons_t trigger;
    Buttons_t hold;
    Buttons_t release;

    static constexpr size_t NUM_BUTTON_BITS = sizeof(Buttons_t) * 8;
    std::array<RepeatInfo, NUM_BUTTON_BITS> repeats;
    std::array<bool, NUM_BUTTON_BITS> setByRepeat;
};
