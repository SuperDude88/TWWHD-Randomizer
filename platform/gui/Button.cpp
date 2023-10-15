#include "Button.hpp"

#include <platform/gui/screen.hpp>
#include <platform/gui/TextWrap.hpp>

bool BasicButton::update(const VPADStatus& stat) {
    if(stat.trigger & VPAD_BUTTON_A) {
        (*cb)();

        return true;
    }

    return false;
}

void BasicButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, name.c_str());
    OSScreenPutFontEx(SCREEN_TV, valCol, row, ("<" + getValue(option) + ">").c_str());
}

void BasicButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, i, descLines[i].c_str());
    }
}


bool CounterButton::update(const VPADStatus& stat) {
    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point next;

    if(stat.trigger & VPAD_BUTTON_A) {
        (*cb)();
        next = Clock::now() + minHold;

        return true;
    }
    else if(stat.hold & VPAD_BUTTON_A) {
        if(Clock::now() >= next) {
            next = Clock::now() + cycleFreq;
            (*cb)();
        }

        return true;
    }

    return false;
}

void CounterButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, name.c_str());
    OSScreenPutFontEx(SCREEN_TV, valCol, row, ("<" + getValue(option) + ">").c_str());
}

void CounterButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, i, descLines[i].c_str());
    }
}
