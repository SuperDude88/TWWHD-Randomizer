#pragma once

#include <string>
#include <optional>
#include <functional>

class KeyboardKey {
private:
    const std::string display;
    const std::function<void(void)> cb;

public:
    KeyboardKey(const std::string& display_, const std::function<void(void)>& cb_) :
        display(display_),
        cb(cb_)
    {}
    
    virtual bool update();
    virtual void drawTV(const size_t row, const size_t col, const bool& selected) const;
    virtual void drawDRC() const;
};

class Keyboard {
protected:
    bool closed = true;
    bool accept = false;
    void setClose(const bool& close_) { closed = close_; }
    void setAccept(const bool& accept_) { accept = accept_; }
    void closeAndAccept() {
        setClose(true);
        setAccept(true);
    }

    std::string input;
public:
    virtual void open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const size_t& max_) = 0;
    virtual bool update() = 0;
    virtual void drawTV(const size_t row, const size_t col) const = 0;
    virtual void drawDRC() const = 0;

    bool isClosed() { return closed; }
    virtual std::optional<std::string> getInput() const;
};

class HexKeyboard : public Keyboard {
private:
    std::string title;
    std::string description;
    size_t max;

    size_t curRow = 0;
    size_t curCol = 0;
    std::array<std::array<KeyboardKey, 3>, 6> keys;

    void addCharacter(const char val);
    void eraseCharacter();
public:
    HexKeyboard();

    void open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const size_t& max_);
    bool update();
    void drawTV(const size_t row, const size_t col) const;
    void drawDRC() const;
};
