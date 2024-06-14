#include "input.hpp"

#include <unordered_map>

#include <command/Log.hpp>

#include <vpad/input.h>
#include <padscore/kpad.h>

static InputError mapVPAD(const VPADStatus& stat, ButtonInfo& out) {
    using enum ButtonInfo::Buttons;

    static const std::unordered_map<VPADButtons, ButtonInfo::Buttons> mapping = {
        {VPAD_BUTTON_A, A},
        {VPAD_BUTTON_B, B},
        {VPAD_BUTTON_X, X},
        {VPAD_BUTTON_Y, Y},
        {VPAD_BUTTON_LEFT, LEFT},
        {VPAD_BUTTON_RIGHT, RIGHT},
        {VPAD_BUTTON_UP, UP},
        {VPAD_BUTTON_DOWN, DOWN},
        {VPAD_BUTTON_ZL, ZL},
        {VPAD_BUTTON_ZR, ZR},
        {VPAD_BUTTON_L, L},
        {VPAD_BUTTON_R, R},
        {VPAD_BUTTON_PLUS, PLUS},
        {VPAD_BUTTON_MINUS, MINUS},
        {VPAD_BUTTON_HOME, HOME},
        {VPAD_BUTTON_SYNC, SYNC},
        {VPAD_BUTTON_STICK_L, STICK_L},
        {VPAD_BUTTON_STICK_R, STICK_R},
        {VPAD_BUTTON_TV, TV},
        {VPAD_STICK_L_EMULATION_LEFT, L_EMULATION_LEFT},
        {VPAD_STICK_L_EMULATION_RIGHT, L_EMULATION_RIGHT},
        {VPAD_STICK_L_EMULATION_UP, L_EMULATION_UP},
        {VPAD_STICK_L_EMULATION_DOWN, L_EMULATION_DOWN},
        {VPAD_STICK_R_EMULATION_LEFT, R_EMULATION_LEFT},
        {VPAD_STICK_R_EMULATION_RIGHT, R_EMULATION_RIGHT},
        {VPAD_STICK_R_EMULATION_UP, R_EMULATION_UP},
        {VPAD_STICK_R_EMULATION_DOWN, R_EMULATION_DOWN}
    };

    for(size_t bitIdx = 0; bitIdx < sizeof(VPADButtons) * 8; bitIdx++) {
        const VPADButtons button = static_cast<VPADButtons>(1 << bitIdx);

        if(stat.trigger & button) {
            if(!mapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.trigger |= mapping.at(button);
        }
        if(stat.hold & button) {
            if(!mapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.hold |= mapping.at(button);
        }
        if(stat.release & button) {
            if(!mapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.release |= mapping.at(button);
        }
    }

    return InputError::NONE;
}

static InputError mapWPAD(const KPADStatus& stat, ButtonInfo& out) {
    using enum ButtonInfo::Buttons;

    static const std::unordered_map<WPADButton, ButtonInfo::Buttons> wiiMapping = {
        {WPAD_BUTTON_A, A},
        {WPAD_BUTTON_B, B},
        {WPAD_BUTTON_C, NUNCHUK_C},
        {WPAD_BUTTON_Z, NUNCHUK_Z},
        {WPAD_BUTTON_1, WII_1},
        {WPAD_BUTTON_2, WII_2},
        {WPAD_BUTTON_LEFT, LEFT},
        {WPAD_BUTTON_RIGHT, RIGHT},
        {WPAD_BUTTON_UP, UP},
        {WPAD_BUTTON_DOWN, DOWN},
        {WPAD_BUTTON_PLUS, PLUS},
        {WPAD_BUTTON_MINUS, MINUS},
        {WPAD_BUTTON_HOME, HOME}
    };
    static const std::unordered_map<WPADNunchukButton, ButtonInfo::Buttons> nunchukMapping = {
        {WPAD_NUNCHUK_BUTTON_C, NUNCHUK_C},
        {WPAD_NUNCHUK_BUTTON_Z, NUNCHUK_Z},
        {WPAD_NUNCHUK_STICK_EMULATION_LEFT, L_EMULATION_LEFT},
        {WPAD_NUNCHUK_STICK_EMULATION_RIGHT, L_EMULATION_RIGHT},
        {WPAD_NUNCHUK_STICK_EMULATION_UP, L_EMULATION_UP},
        {WPAD_NUNCHUK_STICK_EMULATION_DOWN, L_EMULATION_DOWN},
    };
    static const std::unordered_map<WPADClassicButton, ButtonInfo::Buttons> classicMapping = {
        {WPAD_CLASSIC_BUTTON_A, A},
        {WPAD_CLASSIC_BUTTON_B, B},
        {WPAD_CLASSIC_BUTTON_X, X},
        {WPAD_CLASSIC_BUTTON_Y, Y},
        {WPAD_CLASSIC_BUTTON_LEFT, LEFT},
        {WPAD_CLASSIC_BUTTON_RIGHT, RIGHT},
        {WPAD_CLASSIC_BUTTON_UP, UP},
        {WPAD_CLASSIC_BUTTON_DOWN, DOWN},
        {WPAD_CLASSIC_BUTTON_ZL, ZL},
        {WPAD_CLASSIC_BUTTON_ZR, ZR},
        {WPAD_CLASSIC_BUTTON_L, L},
        {WPAD_CLASSIC_BUTTON_R, R},
        {WPAD_CLASSIC_BUTTON_PLUS, PLUS},
        {WPAD_CLASSIC_BUTTON_MINUS, MINUS},
        {WPAD_CLASSIC_BUTTON_HOME, HOME},
        {WPAD_CLASSIC_STICK_L_EMULATION_LEFT, L_EMULATION_LEFT},
        {WPAD_CLASSIC_STICK_L_EMULATION_RIGHT, L_EMULATION_RIGHT},
        {WPAD_CLASSIC_STICK_L_EMULATION_UP, L_EMULATION_UP},
        {WPAD_CLASSIC_STICK_L_EMULATION_DOWN, L_EMULATION_DOWN},
        {WPAD_CLASSIC_STICK_R_EMULATION_LEFT, R_EMULATION_LEFT},
        {WPAD_CLASSIC_STICK_R_EMULATION_RIGHT, R_EMULATION_RIGHT},
        {WPAD_CLASSIC_STICK_R_EMULATION_UP, R_EMULATION_UP},
        {WPAD_CLASSIC_STICK_R_EMULATION_DOWN, R_EMULATION_DOWN}
    };
    static const std::unordered_map<WPADProButton, ButtonInfo::Buttons> proMapping = {
        {WPAD_PRO_BUTTON_A, A},
        {WPAD_PRO_BUTTON_B, B},
        {WPAD_PRO_BUTTON_X, X},
        {WPAD_PRO_BUTTON_Y, Y},
        {WPAD_PRO_BUTTON_LEFT, LEFT},
        {WPAD_PRO_BUTTON_RIGHT, RIGHT},
        {WPAD_PRO_BUTTON_UP, UP},
        {WPAD_PRO_BUTTON_DOWN, DOWN},
        {WPAD_PRO_TRIGGER_ZL, ZL},
        {WPAD_PRO_TRIGGER_ZR, ZR},
        {WPAD_PRO_TRIGGER_L, L},
        {WPAD_PRO_TRIGGER_R, R},
        {WPAD_PRO_BUTTON_PLUS, PLUS},
        {WPAD_PRO_BUTTON_MINUS, MINUS},
        {WPAD_PRO_BUTTON_HOME, HOME},
        {WPAD_PRO_BUTTON_STICK_L, STICK_L},
        {WPAD_PRO_BUTTON_STICK_R, STICK_R},
        {WPAD_PRO_STICK_L_EMULATION_LEFT, L_EMULATION_LEFT},
        {WPAD_PRO_STICK_L_EMULATION_RIGHT, L_EMULATION_RIGHT},
        {WPAD_PRO_STICK_L_EMULATION_UP, L_EMULATION_UP},
        {WPAD_PRO_STICK_L_EMULATION_DOWN, L_EMULATION_DOWN},
        {WPAD_PRO_STICK_R_EMULATION_LEFT, R_EMULATION_LEFT},
        {WPAD_PRO_STICK_R_EMULATION_RIGHT, R_EMULATION_RIGHT},
        {WPAD_PRO_STICK_R_EMULATION_UP, R_EMULATION_UP},
        {WPAD_PRO_STICK_R_EMULATION_DOWN, R_EMULATION_DOWN}
    };

    for(size_t bitIdx = 0; bitIdx < sizeof(WPADButton) * 8; bitIdx++) {
        const WPADButton button = static_cast<WPADButton>(1 << bitIdx);

        if(stat.trigger & button) {
            if(!wiiMapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.trigger |= wiiMapping.at(button);
        }
        if(stat.hold & button) {
            if(!wiiMapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.hold |= wiiMapping.at(button);
        }
        if(stat.release & button) {
            if(!wiiMapping.contains(button)) {
                LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
            }

            out.release |= wiiMapping.at(button);
        }
    }

    switch(stat.extensionType) {
        case WPAD_EXT_CORE:
        case WPAD_EXT_MPLUS:
            break;
        case WPAD_EXT_NUNCHUK:
        case WPAD_EXT_MPLUS_NUNCHUK:
            for(size_t bitIdx = 0; bitIdx < sizeof(WPADNunchukButton) * 8; bitIdx++) {
                const WPADNunchukButton button = static_cast<WPADNunchukButton>(1 << bitIdx);

                if(stat.nunchuk.trigger & button) {
                    if(!nunchukMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.trigger |= nunchukMapping.at(button);
                }
                if(stat.nunchuk.hold & button) {
                    if(!nunchukMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.hold |= nunchukMapping.at(button);
                }
                if(stat.nunchuk.release & button) {
                    if(!nunchukMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.release |= nunchukMapping.at(button);
                }
            }

            break;
        case WPAD_EXT_CLASSIC:
        case WPAD_EXT_MPLUS_CLASSIC:
            for(size_t bitIdx = 0; bitIdx < sizeof(WPADClassicButton) * 8; bitIdx++) {
                const WPADClassicButton button = static_cast<WPADClassicButton>(1 << bitIdx);

                if(stat.classic.trigger & button) {
                    if(!classicMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.trigger |= classicMapping.at(button);
                }
                if(stat.classic.hold & button) {
                    if(!classicMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.hold |= classicMapping.at(button);
                }
                if(stat.classic.release & button) {
                    if(!classicMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.release |= classicMapping.at(button);
                }
            }

            break;
        case WPAD_EXT_PRO_CONTROLLER:
            for(size_t bitIdx = 0; bitIdx < sizeof(WPADProButton) * 8; bitIdx++) {
                const WPADProButton button = static_cast<WPADProButton>(1 << bitIdx);

                if(stat.pro.trigger & button) {
                    if(!proMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.trigger |= proMapping.at(button);
                }
                if(stat.pro.hold & button) {
                    if(!proMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.hold |= proMapping.at(button);
                }
                if(stat.pro.release & button) {
                    if(!proMapping.contains(button)) {
                        LOG_ERR_AND_RETURN(InputError::INVALID_MAPPING);
                    }

                    out.release |= proMapping.at(button);
                }
            }

            break;
        default:
            LOG_ERR_AND_RETURN(InputError::INVALID_CONTROLLER);
    }

    return InputError::NONE;
}

InputManager::InputManager() {
    VPADInit();
    KPADInit();
    WPADEnableURCC(true);
    WPADEnableWiiRemote(true);
}

InputManager& InputManager::getInstance() {
    static InputManager sInstance;

    return sInstance;
}

InputError InputManager::poll() {
    ButtonInfo readInfo;
    bool read = false;
    
    for(const VPADChan channel : {VPAD_CHAN_0, VPAD_CHAN_1}) {
        VPADStatus status{};
        VPADReadError err;
        if(VPADRead(channel, &status, 1, &err) != 0 && err == VPAD_READ_SUCCESS) {
            read = true;
            LOG_AND_RETURN_IF_ERR(mapVPAD(status, readInfo));
        }
        else if(err == VPAD_READ_UNINITIALIZED) {
            LOG_ERR_AND_RETURN(InputError::UNINITIALIZED);
        }
    }
    
    for(const KPADChan channel : {WPAD_CHAN_0, WPAD_CHAN_1, WPAD_CHAN_2, WPAD_CHAN_3}) {
        KPADStatus status{};
        KPADError err;
        if(KPADReadEx(channel, &status, 1, &err) != 0 && err == KPAD_ERROR_OK) {
            read = true;
            LOG_AND_RETURN_IF_ERR(mapWPAD(status, readInfo));
        }
        else if(err == KPAD_ERROR_UNINITIALIZED || err == KPAD_ERROR_WPAD_UNINIT) {
            LOG_ERR_AND_RETURN(InputError::UNINITIALIZED);
        }
    }

    if(!read) {
        LOG_ERR_AND_RETURN(InputError::NO_CONTROLLER_READ);
    }

    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const ButtonInfo::Buttons button = static_cast<ButtonInfo::Buttons>(1 << bitIdx);
        RepeatInfo& repeat = repeats[bitIdx];

        if(readInfo.trigger & button) {
            current.trigger |= button;
            current.hold &= ~button;
            current.release &= ~button;

            setByRepeat[bitIdx] = false;
            repeat.setLastPressed(Clock_t::now());
        }
        else if(readInfo.hold & button) {
            if(repeat.doRepeat()) {
                current.trigger &= ~button;
                current.hold &= ~button;

                if(setByRepeat[bitIdx]) {
                    current.release |= button;
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
                            current.trigger |= button;
                            current.hold &= ~button;
                            current.release &= ~button;

                            repeat.setLastPressed(Clock_t::now());
                            setByRepeat[bitIdx] = true;
                        }
                    }
                }
            }
            else {
              current.trigger &= ~button;
              current.hold |= button;
              current.release &= ~button;
            }
        }
        else if(readInfo.release & button) {
            current.trigger &= ~button;
            current.hold &= ~button;
            current.release |= button;

            repeat.setWait(true);
        }
    }

    return InputError::NONE;
}

bool InputManager::pressed(const Buttons_t& buttons) {
    return current.trigger & buttons;
}

bool InputManager::held(const Buttons_t& buttons) {
    return current.hold & buttons;
}

bool InputManager::released(const Buttons_t& buttons) {
    return current.release & buttons;
}

void InputManager::setRepeat(const Buttons_t& buttons, const Duration_t& delay, const Duration_t& interval) {
    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const Buttons_t button = static_cast<Buttons_t>(1 << bitIdx);
        if(buttons & button) {
            repeats[bitIdx] = RepeatInfo{delay, interval};
            setByRepeat[bitIdx] = false;
        }
    }
}

void InputManager::clearRepeat(const Buttons_t& buttons) {
    for(size_t bitIdx = 0; bitIdx < NUM_BUTTON_BITS; bitIdx++) {
        const Buttons_t button = static_cast<Buttons_t>(1 << bitIdx);
        if(buttons & button) {
            repeats[bitIdx] = RepeatInfo{};
            setByRepeat[bitIdx] = false;
        }
    }
}

