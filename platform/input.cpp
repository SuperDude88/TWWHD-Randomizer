#include "input.hpp"

InputManager& InputManager::getInstance() {
    static InputManager sInstance;

    return sInstance;
}

InputError InputManager::poll() {
    VPADStatus status{};
    VPADReadError err;
    if(VPADRead(VPAD_CHAN_0, &status, 1, &err) != 0 && err != VPAD_READ_SUCCESS) {
        switch(err) {
            case VPAD_READ_NO_SAMPLES:
                return InputError::NO_SAMPLES;
            case VPAD_READ_INVALID_CONTROLLER:
                return InputError::INVALID_CONTROLLER;
            case VPAD_READ_BUSY:
                return InputError::BUSY;
            case VPAD_READ_UNINITIALIZED:
                return InputError::UNINITIALIZED;
            default:
                return InputError::UNKNOWN;
        }
    }

    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const Buttons_t button = 0x00000001 << bitIdx;
        RepeatInfo& repeat = repeats[bitIdx];

        if(status.trigger & button) {
            trigger |= button;
            hold &= ~button;
            release &= ~button;

            setByRepeat[bitIdx] = false;
            repeat.setLastPressed(Clock_t::now());
        }
        else if(status.hold & button) {
            if(repeat.doRepeat()) {
                trigger &= ~button;
                hold &= ~button;

                if(setByRepeat[bitIdx]) {
                    release |= button;
                    setByRepeat[bitIdx] = false;
                }
                else {
                    const Duration_t sincePressed = Clock_t::now() - repeat.getLastPressed();
                    if(repeat.doWait()) {
                        if(sincePressed > repeat.getDelay()) {
                            repeat.setWait(false);
                        }
                    }

                    if(!repeat.doWait()) {
                        if(sincePressed > repeat.getInterval()) {
                            trigger |= button;
                            hold &= ~button;
                            release &= ~button;

                            repeat.setLastPressed(Clock_t::now());
                            setByRepeat[bitIdx] = true;
                        }
                    }
                }
            }
            else {
              trigger &= ~button;
              hold |= button;
              release &= ~button;
            }
        }
        else if(status.release & button) {
            trigger &= ~button;
            hold &= ~button;
            release |= button;

            repeat.setWait(true);
        }
    }

    return InputError::NONE;
}

bool InputManager::pressed(const VPADButtons& buttons) {
    return trigger & buttons;
}

bool InputManager::held(const VPADButtons& buttons) {
    return hold & buttons;
}

bool InputManager::released(const VPADButtons& buttons) {
    return release & buttons;
}

void InputManager::setRepeat(const VPADButtons& buttons, const Duration_t& delay, const Duration_t& interval) {
    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const Buttons_t button = 0x00000001 << bitIdx;
        if(buttons & button) {
            repeats[bitIdx] = RepeatInfo{delay, interval};
            setByRepeat[bitIdx] = false;
        }
    }
}

void InputManager::clearRepeat(const VPADButtons& buttons) {
    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const Buttons_t button = 0x00000001 << bitIdx;
        if(buttons & button) {
            repeats[bitIdx] = RepeatInfo{};
            setByRepeat[bitIdx] = false;
        }
    }
}

