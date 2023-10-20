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
    std::array<std::unique_ptr<EmptyPage>, 7> pages;

    SettingsMenu();
    ~SettingsMenu() {}

    Status update();
    void drawTV() const;
    void drawDRC() const;
};
