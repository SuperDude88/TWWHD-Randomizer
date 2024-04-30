#pragma once

#include <string>
#include <optional>
#include <functional>

class KeyboardKey {
private:
    const std::string display;
    const std::function<void(void)> cb;

public:
    KeyboardKey(const char& display_, const std::function<void(void)>& cb_) :
        KeyboardKey(std::string(1, display_), cb_)
    {}
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
    std::string title = "";
    std::string description = "";
    std::optional<size_t> max = std::nullopt;

    bool closed = true;
    bool accept = false;
    void setClose(const bool& close_) { closed = close_; }
    void setAccept(const bool& accept_) { accept = accept_; }
    void closeAndAccept() {
        setClose(true);
        setAccept(true);
    }

    std::string input;
    virtual void addCharacter(const std::string::value_type val);
    virtual void eraseCharacter();

public:
    Keyboard() = default;
    virtual ~Keyboard() = default;

    virtual void open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const std::optional<size_t>& max_) = 0;
    virtual bool update() = 0;
    virtual void drawTV(const size_t row, const size_t col) const = 0;
    virtual void drawDRC() const = 0;

    bool isClosed() { return closed; }
    virtual std::optional<std::string> getInput() const;
};

class HexKeyboard final : public Keyboard {
private:
    size_t curRow = 0;
    size_t curCol = 0;
    std::array<std::array<KeyboardKey, 3>, 6> keys;

public:
    HexKeyboard();
    ~HexKeyboard() override = default;

    void open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const std::optional<size_t>& max_) override;
    bool update() override;
    void drawTV(const size_t row, const size_t col) const override;
    void drawDRC() const override;
};

class USKeyboard final : public Keyboard {
private:
    enum Keyset {
        LOWERCASE = 0,
        UPPERCASE,
        SPECIAL
    };

    size_t curRow = 0;
    size_t curCol = 0;
    Keyset curKeys = Keyset::LOWERCASE;
    std::array<std::array<KeyboardKey, 10>, 3> lowercase;
    std::array<std::array<KeyboardKey, 10>, 3> uppercase;
    std::array<std::array<KeyboardKey, 10>, 5> special;

    void setKeys(const Keyset& keys_) {
        curKeys = keys_;
        switch(curKeys) { // keep cursor on keyboard switch key
            case LOWERCASE:
                curRow = 2;
                curCol = 8;
                break;
            case UPPERCASE:
                curRow = 2;
                curCol = 8;
                break;
            case SPECIAL:
                curRow = 4;
                curCol = 8;
                break;
        }
    }

public:
    USKeyboard();
    ~USKeyboard() override = default;

    void open(const std::string& title_, const std::string& desc_, const std::string& curVal_, const std::optional<size_t>& max_) override;
    bool update() override;
    void drawTV(const size_t row, const size_t col) const override;
    void drawDRC() const override;
};
