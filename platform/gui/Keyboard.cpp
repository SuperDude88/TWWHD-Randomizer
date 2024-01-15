#include "Keyboard.hpp"

#include <platform/gui/screen.hpp>

bool KeyboardKey::update(const VPADStatus& stat) {
    if(stat.trigger & VPAD_BUTTON_A) {
        cb();

        return true;
    }

    return false;
}

void KeyboardKey::drawTV(const size_t row, const size_t col, const bool& selected) const {
    size_t start = col;
    if(selected) {
        start -= 1;
        OSScreenPutFontEx(SCREEN_TV, start, row, ('>' + display + '<').c_str());
    }
    else {
        OSScreenPutFontEx(SCREEN_TV, start, row, display.c_str());
    }
}

void KeyboardKey::drawDRC() const {}

std::optional<std::string> Keyboard::getInput() const {
    if(accept) {
        return input;
    }
    else {
        return std::nullopt;
    }
}

void HexKeyboard::addCharacter(const char val) {
    if(input.length() < max) {
        input += val;
    }
}

void HexKeyboard::eraseCharacter() {
    if(input.length() > 0) {
        input.pop_back();
    }
}

HexKeyboard::HexKeyboard() :
    keys{{
        {KeyboardKey("D", std::bind(&HexKeyboard::addCharacter, this, 'D')), KeyboardKey("E", std::bind(&HexKeyboard::addCharacter, this, 'E')), KeyboardKey("F", std::bind(&HexKeyboard::addCharacter, this, 'F'))},
        {KeyboardKey("A", std::bind(&HexKeyboard::addCharacter, this, 'A')), KeyboardKey("B", std::bind(&HexKeyboard::addCharacter, this, 'B')), KeyboardKey("C", std::bind(&HexKeyboard::addCharacter, this, 'C'))},
        {KeyboardKey("7", std::bind(&HexKeyboard::addCharacter, this, '7')), KeyboardKey("8", std::bind(&HexKeyboard::addCharacter, this, '8')), KeyboardKey("9", std::bind(&HexKeyboard::addCharacter, this, '9'))},
        {KeyboardKey("4", std::bind(&HexKeyboard::addCharacter, this, '4')), KeyboardKey("5", std::bind(&HexKeyboard::addCharacter, this, '5')), KeyboardKey("6", std::bind(&HexKeyboard::addCharacter, this, '6'))},
        {KeyboardKey("1", std::bind(&HexKeyboard::addCharacter, this, '1')), KeyboardKey("2", std::bind(&HexKeyboard::addCharacter, this, '2')), KeyboardKey("3", std::bind(&HexKeyboard::addCharacter, this, '3'))},
        {KeyboardKey("<-", std::bind(&HexKeyboard::eraseCharacter, this)), KeyboardKey("0", std::bind(&HexKeyboard::addCharacter, this, '0')), KeyboardKey("OK", std::bind(&HexKeyboard::closeAndAccept, this))},
    }}
{}

void HexKeyboard::open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const size_t& max_) {
    title = title_;
    description = desc_;
    input = curVal_;
    max = max_;

    setClose(false);
    setAccept(false);

    curRow = 0;
    curCol = 0;
    
    for(char& c : input) {
        c = std::toupper(c); //capitalize for consistency
    }

    closed = false;
}

bool HexKeyboard::update(const VPADStatus& stat) {
    if(stat.trigger & VPAD_BUTTON_B) {
        setClose(true);
        setAccept(false);

        return true;
    }

    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = keys[0].size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= keys[0].size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = keys.size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= keys.size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || keys[curRow][curCol].update(stat);
}

void HexKeyboard::drawTV(const size_t row, const size_t col) const {
    OSScreenPutFontEx(SCREEN_TV, col, row, title.c_str());
    
    OSScreenPutFontEx(SCREEN_TV, col, row + 2, input.c_str());

    for(size_t i = 0; i < keys.size(); i++) {
        for(size_t j = 0; j < keys[i].size(); j++) {
            keys[i][j].drawTV(row + 4 + i, col + j * 4, i == curRow && j == curCol);
        }
    }
}

void HexKeyboard::drawDRC() const {
    OSScreenPutFontEx(SCREEN_TV, 0, 0, description.c_str());

    keys[curRow][curCol].drawDRC();
}
