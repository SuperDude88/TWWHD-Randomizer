#pragma once

#include <array>
#include <chrono>

enum struct [[nodiscard]] InputError {
    NONE = 0,
    INVALID_CONTROLLER,
    INVALID_MAPPING,
    NO_CONTROLLER_READ,
    UNINITIALIZED,
    UNKNOWN,
    COUNT
};

struct ButtonInfo {
    enum Buttons {
        A = 1 << 0,
        B = 1 << 1,
        X = 1 << 2,
        Y = 1 << 3,
        LEFT = 1 << 4,
        RIGHT = 1 << 5,
        UP = 1 << 6,
        DOWN = 1 << 7,
        ZL = 1 << 8,
        ZR = 1 << 9,
        L = 1 << 10,
        R = 1 << 11,
        PLUS = 1 << 12,
        MINUS = 1 << 13,
        HOME = 1 << 14,
        SYNC = 1 << 15,
        STICK_L = 1 << 16,
        STICK_R = 1 << 17,
        TV = 1 << 18,
        L_EMULATION_LEFT = 1 << 19,
        L_EMULATION_RIGHT = 1 << 20,
        L_EMULATION_UP = 1 << 21,
        L_EMULATION_DOWN = 1 << 22,
        R_EMULATION_LEFT = 1 << 23,
        R_EMULATION_RIGHT = 1 << 24,
        R_EMULATION_UP = 1 << 25,
        R_EMULATION_DOWN = 1 << 26,
        WII_1 = 1 << 27,
        WII_2 = 1 << 28,
        NUNCHUK_Z = 1 << 29,
        NUNCHUK_C = 1 << 30
    };
    
    uint32_t trigger = 0;
    uint32_t hold = 0;
    uint32_t release = 0;
};

class InputManager {
private:
    InputManager();
    ~InputManager() {}

public:
    using Clock_t = std::chrono::high_resolution_clock;
    using Duration_t = Clock_t::duration;
    using Time_t = Clock_t::time_point;
    
    using Buttons_t = ButtonInfo::Buttons;

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
    bool pressed(const Buttons_t& buttons);
    bool held(const Buttons_t& buttons);
    bool released(const Buttons_t& buttons);
    void setRepeat(const Buttons_t& buttons, const Duration_t& delay, const Duration_t& interval);
    void clearRepeat(const Buttons_t& buttons);

private:
    ButtonInfo current;

    static constexpr size_t NUM_BUTTON_BITS = sizeof(ButtonInfo::Buttons) * 8;
    std::array<RepeatInfo, NUM_BUTTON_BITS> repeats;
    std::array<bool, NUM_BUTTON_BITS> setByRepeat;
};
