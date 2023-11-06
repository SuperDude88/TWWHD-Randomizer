#pragma once

#include <platform/gui/Page.hpp>

class SettingsMenu {
public:
    enum class Status {
        NONE = 0,
        CHANGED = 1,
        EXIT = 2
    };

    SettingsMenu(const SettingsMenu&) = delete;
    SettingsMenu& operator=(const SettingsMenu&) = delete;

    static bool run(Config& out);

private:
    size_t curPage = 0;
    std::array<std::unique_ptr<EmptyPage>, 9> pages;

    SettingsMenu();
    ~SettingsMenu() {}

    static SettingsMenu& getInstance();

    Status update();
    void drawTV() const;
    void drawDRC() const;

    static uint32_t acquireCB(void* = nullptr);
    static uint32_t releaseCB(void* = nullptr);
};
