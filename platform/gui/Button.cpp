#include "Button.hpp"

#include <platform/gui/screen.hpp>
#include <platform/gui/TextWrap.hpp>

void BasicButton::hovered() {
    InputManager::getInstance().setRepeat(ButtonInfo::A, delay, interval);
}

void BasicButton::unhovered() {
    InputManager::getInstance().clearRepeat(ButtonInfo::A);
}

bool BasicButton::update() {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
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



bool ItemButton::operator==(const ItemButton& rhs) const {
    return gameItemToName(item) == gameItemToName(rhs.item);
}

bool ItemButton::operator==(const GameItem& rhs) const {
    return gameItemToName(item) == gameItemToName(rhs);
}

bool ItemButton::update() {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        enabled = !enabled;

        return true;
    }

    return false;
}

void ItemButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, gameItemToName(item).c_str());

    const std::string mark = enabled ? "X" : "";
    OSScreenPutFontEx(SCREEN_TV, valCol, row, mark.c_str());
}

void ItemButton::drawDRC() const {
    
}


bool ActionButton::update() {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        (*cb)();

        return true;
    }

    return false;
}

void ActionButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, name.c_str());

    if(valueCB != &OptionCB::invalidCB) {
        OSScreenPutFontEx(SCREEN_TV, valCol, row, ("<" + (*valueCB)() + ">").c_str());
    }
}

void ActionButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, i, descLines[i].c_str());
    }
}


bool ColorButton::update(const std::string& colorName) {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        colorCB(colorName);

        return true;
    }

    return false;
}

void ColorButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, name.c_str());
}

void ColorButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, i, descLines[i].c_str());
    }
}


bool FunctionButton::update() {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        triggerCB();

        return true;
    }

    return false;
}

void FunctionButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, name.c_str());
}

void FunctionButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, i, descLines[i].c_str());
    }
}
