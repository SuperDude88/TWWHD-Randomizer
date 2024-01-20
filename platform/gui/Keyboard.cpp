#include "Keyboard.hpp"

#include <platform/input.hpp>
#include <platform/gui/screen.hpp>

bool KeyboardKey::update() {
    if(InputManager::getInstance().pressed(VPAD_BUTTON_A)) {
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

void Keyboard::addCharacter(const std::string::value_type val) {
    if(!max.has_value() || input.length() < max.value()) {
        input += val;
    }
}

void Keyboard::eraseCharacter() {
    if(input.length() > 0) {
        input.pop_back();
    }
}

std::optional<std::string> Keyboard::getInput() const {
    if(accept) {
        return input;
    }
    else {
        return std::nullopt;
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

void HexKeyboard::open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const std::optional<size_t>& max_) {
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

bool HexKeyboard::update() {
    if(InputManager::getInstance().pressed(VPAD_BUTTON_B)) {
        setClose(true);
        setAccept(false);

        return true;
    }

    bool moved = false;

    if(InputManager::getInstance().pressed(VPAD_BUTTON_LEFT)) {
        if(curCol <= 0) {
            curCol = keys[0].size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(VPAD_BUTTON_RIGHT)) {
        if(curCol >= keys[0].size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(InputManager::getInstance().pressed(VPAD_BUTTON_UP)) {
        if(curRow <= 0) {
            curRow = keys.size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(VPAD_BUTTON_DOWN)) {
        if(curRow >= keys.size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || keys[curRow][curCol].update();
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

#define SIMPLE_KEY(character) KeyboardKey(character, std::bind(&USKeyboard::addCharacter, this, character))

USKeyboard::USKeyboard() :
    lowercase{{
        {SIMPLE_KEY('q'), SIMPLE_KEY('w'), SIMPLE_KEY('e'), SIMPLE_KEY('r'), SIMPLE_KEY('t'), SIMPLE_KEY('y'), SIMPLE_KEY('u'), SIMPLE_KEY('i'), SIMPLE_KEY('o'), SIMPLE_KEY('p')},
        {SIMPLE_KEY('a'), SIMPLE_KEY('s'), SIMPLE_KEY('d'), SIMPLE_KEY('f'), SIMPLE_KEY('g'), SIMPLE_KEY('h'), SIMPLE_KEY('j'), SIMPLE_KEY('k'), SIMPLE_KEY('l'), KeyboardKey("<-", std::bind(&USKeyboard::eraseCharacter, this))},
        {SIMPLE_KEY('z'), SIMPLE_KEY('x'), SIMPLE_KEY('c'), SIMPLE_KEY('v'), SIMPLE_KEY('b'), SIMPLE_KEY('n'), SIMPLE_KEY('m'), SIMPLE_KEY(' '), KeyboardKey("^A", std::bind(&USKeyboard::setKeys, this, Keyset::UPPERCASE)), KeyboardKey("OK", std::bind(&USKeyboard::closeAndAccept, this))}
    }},
    uppercase{{
        {SIMPLE_KEY('Q'), SIMPLE_KEY('W'), SIMPLE_KEY('E'), SIMPLE_KEY('R'), SIMPLE_KEY('T'), SIMPLE_KEY('Y'), SIMPLE_KEY('U'), SIMPLE_KEY('I'), SIMPLE_KEY('O'), SIMPLE_KEY('P')},
        {SIMPLE_KEY('A'), SIMPLE_KEY('S'), SIMPLE_KEY('D'), SIMPLE_KEY('F'), SIMPLE_KEY('G'), SIMPLE_KEY('H'), SIMPLE_KEY('J'), SIMPLE_KEY('K'), SIMPLE_KEY('L'), KeyboardKey("<-", std::bind(&USKeyboard::eraseCharacter, this))},
        {SIMPLE_KEY('Z'), SIMPLE_KEY('X'), SIMPLE_KEY('C'), SIMPLE_KEY('V'), SIMPLE_KEY('B'), SIMPLE_KEY('N'), SIMPLE_KEY('M'), SIMPLE_KEY(' '), KeyboardKey("^1", std::bind(&USKeyboard::setKeys, this, Keyset::SPECIAL)), KeyboardKey("OK", std::bind(&USKeyboard::closeAndAccept, this))}
    }},
    special{{ // have a few extra spaces but there's not any(?) more standard ASCII characters to use
        {SIMPLE_KEY('1'), SIMPLE_KEY('2'), SIMPLE_KEY('3'), SIMPLE_KEY('4'), SIMPLE_KEY('5'), SIMPLE_KEY('6'), SIMPLE_KEY('7'), SIMPLE_KEY('8'), SIMPLE_KEY('9'), SIMPLE_KEY('0')},
        {SIMPLE_KEY('!'), SIMPLE_KEY('@'), SIMPLE_KEY('#'), SIMPLE_KEY('$'), SIMPLE_KEY('%'), SIMPLE_KEY('^'), SIMPLE_KEY('&'), SIMPLE_KEY('*'), SIMPLE_KEY('('), SIMPLE_KEY(')')},
        {SIMPLE_KEY('`'), SIMPLE_KEY('~'), SIMPLE_KEY('-'), SIMPLE_KEY('_'), SIMPLE_KEY('='), SIMPLE_KEY('+'), SIMPLE_KEY(';'), SIMPLE_KEY(':'), SIMPLE_KEY('['), SIMPLE_KEY(']')},
        {SIMPLE_KEY(','), SIMPLE_KEY('.'), SIMPLE_KEY('<'), SIMPLE_KEY('>'), SIMPLE_KEY('/'), SIMPLE_KEY('?'), SIMPLE_KEY('{'), SIMPLE_KEY('}'), SIMPLE_KEY(' '), KeyboardKey("<-", std::bind(&USKeyboard::eraseCharacter, this))},
        {SIMPLE_KEY('\''), SIMPLE_KEY('\"'), SIMPLE_KEY('|'), SIMPLE_KEY('\\'), SIMPLE_KEY(' '), SIMPLE_KEY(' '), SIMPLE_KEY(' '), SIMPLE_KEY(' '), KeyboardKey("^a", std::bind(&USKeyboard::setKeys, this, Keyset::LOWERCASE)), KeyboardKey("OK", std::bind(&USKeyboard::closeAndAccept, this))}
    }}
{}

void USKeyboard::open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const std::optional<size_t>& max_) {
    title = title_;
    description = desc_;
    input = curVal_;
    max = max_;

    setClose(false);
    setAccept(false);

    curRow = 0;
    curCol = 0;
    curKeys = Keyset::LOWERCASE;

    closed = false;
}

bool USKeyboard::update() {
    if(InputManager::getInstance().pressed(VPAD_BUTTON_B)) {
        setClose(true);
        setAccept(false);

        return true;
    }

    size_t numRows = 0;
    std::array<KeyboardKey, 10>* rows = nullptr;
    switch(curKeys) {
        case LOWERCASE:
            numRows = lowercase.size();
            rows = lowercase.data();
            break;
        case UPPERCASE:
            numRows = uppercase.size();
            rows = uppercase.data();
            break;
        case SPECIAL:
            numRows = special.size();
            rows = special.data();
            break;
    }

    bool moved = false;
    if(InputManager::getInstance().pressed(VPAD_BUTTON_LEFT)) {
        if(curCol <= 0) {
            curCol = rows[0].size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(VPAD_BUTTON_RIGHT)) {
        if(curCol >= rows[0].size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(InputManager::getInstance().pressed(VPAD_BUTTON_UP)) {
        if(curRow <= 0) {
            curRow = numRows - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(VPAD_BUTTON_DOWN)) {
        if(curRow >= numRows - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || rows[curRow][curCol].update();
}

void USKeyboard::drawTV(const size_t row, const size_t col) const {
    OSScreenPutFontEx(SCREEN_TV, col, row, title.c_str());
    
    OSScreenPutFontEx(SCREEN_TV, col, row + 2, input.c_str());
    
    size_t numRows = 0;
    const std::array<KeyboardKey, 10>* rows = nullptr;
    switch(curKeys) {
        case LOWERCASE:
            numRows = lowercase.size();
            rows = lowercase.data();
            break;
        case UPPERCASE:
            numRows = uppercase.size();
            rows = uppercase.data();
            break;
        case SPECIAL:
            numRows = special.size();
            rows = special.data();
            break;
    }

    for(size_t i = 0; i < numRows; i++) {
        for(size_t j = 0; j < rows[i].size(); j++) {
            rows[i][j].drawTV(row + 4 + i, col + j * 4, i == curRow && j == curCol);
        }
    }
}

void USKeyboard::drawDRC() const {
    OSScreenPutFontEx(SCREEN_TV, 0, 0, description.c_str());

    const std::array<KeyboardKey, 10>* rows = nullptr;
    switch(curKeys) {
        case LOWERCASE:
            rows = lowercase.data();
            break;
        case UPPERCASE:
            rows = uppercase.data();
            break;
        case SPECIAL:
            rows = special.data();
            break;
    }
    rows[curRow][curCol].drawDRC();
}
