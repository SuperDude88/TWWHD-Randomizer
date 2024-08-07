#pragma once

#include <platform/gui/Page.hpp>

class SettingsMenu {
public:
    enum struct Status {
        NONE = 0,
        CHANGED,
        EXIT
    };

    enum struct Result {
        CONTINUE = 0,
        EXIT,
        CONFIG_SAVE_FAILED
    };

    SettingsMenu(const SettingsMenu&) = delete;
    SettingsMenu& operator=(const SettingsMenu&) = delete;

    static Result run(Config& out);

private:
    size_t curPage = 0;
    std::array<std::unique_ptr<EmptyPage>, 10> pages;

    SettingsMenu();
    ~SettingsMenu() = default;

    static SettingsMenu& getInstance();

    Status update();
    void drawTV() const;
    void drawDRC() const;

    static uint32_t acquireCB(void* = nullptr);
    static uint32_t releaseCB(void* = nullptr);
};
