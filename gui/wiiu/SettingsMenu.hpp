#pragma once

#include <gui/wiiu/Page.hpp>

class SettingsMenu {
public:
    enum struct Result {
        CONTINUE = 0,
        EXIT,
        CONFIG_ERROR
    };

    SettingsMenu(const SettingsMenu&) = delete;
    SettingsMenu& operator=(const SettingsMenu&) = delete;

    static Result run();

private:
    enum struct Status {
        NONE = 0,
        CHANGED,
        EXIT
    };

    size_t curPage = 0;
    std::array<std::unique_ptr<EmptyPage>, 10> pages;

    SettingsMenu();
    ~SettingsMenu() = default;

    static SettingsMenu& getInstance();

    Status update();
    void draw() const;
    void drawTV() const;
    void drawDRC() const;

    static uint32_t acquireCB(void* = nullptr);
    static uint32_t releaseCB(void* = nullptr);
};
