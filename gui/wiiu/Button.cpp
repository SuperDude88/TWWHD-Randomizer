#include "Button.hpp"

#include <gui/wiiu/screen.hpp>
#include <gui/wiiu/TextWrap.hpp>

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
        OSScreenPutFontEx(SCREEN_DRC, 0, 2 + i, descLines[i].c_str());
    }
}

bool ModelButton::update() {
    if (InputManager::getInstance().pressed(ButtonInfo::A)) {
        //Handle separatly the cases where we clicked the "Link" button and the "Random Model" button
        //I should probably make it their own attributes but i'm lazy so we'll just hardcode it
        if (modelName == "Link") {
            getModel().modelName = "";
            getModel().user_provided = false;
        }
        else if (modelName == "Random Model") {
            getModel().modelName = "random";
            getModel().user_provided = true;
        }
        else {
            getModel().modelName = modelName;
            getModel().user_provided = true;
        }
        return true;
    }
    return false;
}

void ModelButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, modelName.c_str());

    const std::string mark = enabled ? "X" : "";
    OSScreenPutFontEx(SCREEN_TV, valCol, row, mark.c_str());
}

void ModelButton::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(description, ScreenSizeData::drc_line_length);
    for (size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, 2 + i, descLines[i].c_str());
    }
}

void ModelButton::setEnabled(bool value) {
    enabled = value;
}

bool ItemButton::operator==(const ItemButton& rhs) const {
    return item == rhs.item && num == rhs.num;
}

bool ItemButton::operator==(const GameItem& rhs) const {
    return item == rhs;
}

bool ItemButton::operator<(const ItemButton& rhs) const {
    // first check alphabetical ordering
    if(gameItemToName(item) < gameItemToName(rhs.item)) {
        return true;
    }

    // if the items are the same, then check number of duplicates required
    if(item == rhs.item && num < rhs.num) {
        return true;
    }

    return false; // was later in the alphabet or required more duplicates
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

bool LocationButton::operator==(const LocationButton& rhs) const {
    return locName == rhs.locName;
}

bool LocationButton::operator==(const std::string& rhs) const {
    return locName == rhs;
}

bool LocationButton::operator<(const LocationButton& rhs) const {
    // check alphabetical ordering
    if(locName < rhs.locName) {
        return true;
    }

    return false; // was later in the alphabet
}

bool LocationButton::update() {
    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        enabled = !enabled;

        return true;
    }

    return false;
}

void LocationButton::drawTV(const size_t row, const size_t nameCol, const size_t valCol) const {
    OSScreenPutFontEx(SCREEN_TV, nameCol, row, locName.c_str());

    const std::string mark = enabled ? "X" : "";
    OSScreenPutFontEx(SCREEN_TV, valCol, row, mark.c_str());
}

void LocationButton::drawDRC() const {
    
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
        OSScreenPutFontEx(SCREEN_DRC, 0, 2 + i, descLines[i].c_str());
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
        OSScreenPutFontEx(SCREEN_DRC, 0, 2 + i, descLines[i].c_str());
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
        OSScreenPutFontEx(SCREEN_DRC, 0, 2 + i, descLines[i].c_str());
    }
}
