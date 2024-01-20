#include "SettingsMenu.hpp"

#include <thread>
#include <chrono>

#include <version.hpp>
#include <utility/platform.hpp>
#include <platform/proc.hpp>
#include <platform/home.hpp>
#include <platform/energy_saver.hpp>
#include <platform/input.hpp>
#include <platform/gui/screen.hpp>

static const std::string line_break(ScreenSizeData::tv_line_length, '=');

SettingsMenu::SettingsMenu() {
    pages[0] = std::make_unique<SeedPage>();
    pages[1] = std::make_unique<ProgressionPage>();
    pages[2] = std::make_unique<HintsPage>();
    pages[3] = std::make_unique<EntrancePage>();
    pages[4] = std::make_unique<ConveniencePage>();
    pages[5] = std::make_unique<AdvancedPage>();
    pages[6] = std::make_unique<ItemsPage>();
    pages[7] = std::make_unique<ColorPage>();
    pages[8] = std::make_unique<MetaPage>();
}


SettingsMenu& SettingsMenu::getInstance() {
    static SettingsMenu sInstance;
    return sInstance;
}

SettingsMenu::Status SettingsMenu::update() {
    if(InputManager::getInstance().poll() != InputError::NONE) {
        return Status::NONE;
    }

    if(InputManager::getInstance().pressed(VPAD_BUTTON_PLUS)) {
        pages[curPage]->close();

        return Status::EXIT;
    }

    bool moved = false;
    if(InputManager::getInstance().pressed(VPAD_BUTTON_ZL)) {
        pages[curPage]->close();

        if(curPage <= 0) {
            curPage = pages.size() - 1; //wrap on leftmost page
        }
        else {
            curPage -= 1; //left one page
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(VPAD_BUTTON_ZR)) {
        pages[curPage]->close();

        if(curPage >= pages.size() - 1) {
            curPage = 0; //wrap on rightmost page
        }
        else {
            curPage += 1; //right one page
        }
        moved = true;
    }

    if(moved) {
        pages[curPage]->open();
    }
    
    return (moved || pages[curPage]->update()) ? Status::CHANGED : Status::NONE;
}

void SettingsMenu::drawTV() const {
    std::string header;
    for(size_t i = 0; i < pages.size(); i++) {
        if(i == curPage) {
            header += '<' + pages[i]->getName() + '>';
        }
        else {
            header += ' ' + pages[i]->getName() + ' ';
        }

        //not the last item
        if(i + 1 != pages.size()) {
            header += '|';
        }
    }

    OSScreenPutFontEx(SCREEN_TV, 0, 0, header.c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, 1, line_break.c_str());
    
    pages[curPage]->drawTV();

    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 3, line_break.c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 2, "ZL Left Page | A Toggle Option | DPad Navigate | ZR Right Page | Start (+) to Randomize");
    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 1, "TWWHD Randomizer Version " RANDOMIZER_VERSION);
}

void SettingsMenu::drawDRC() const {
    pages[curPage]->drawDRC();
}

uint32_t SettingsMenu::acquireCB(void*) {
    // This prevents an occasional (temporary) black screen when exiting the home overlay
    // I have no idea why
    ScreenClear();
    ScreenDraw();

    ScreenClear();
    getInstance().drawTV();
    getInstance().drawDRC();
    ScreenDraw();

    return 0;
}

uint32_t SettingsMenu::releaseCB(void*) {
    return 0;
}

SettingsMenu::Result SettingsMenu::run(Config& out) {
    using namespace std::literals::chrono_literals;

    SettingsMenu& sInstance = getInstance();

    // void* arg is in the place of "this" pointer
    ScopedCallback procCB({2, &SettingsMenu::acquireCB, &SettingsMenu::releaseCB});
    acquireCB();

    OptionCB::setInternal(out);
    sInstance.pages[sInstance.curPage]->open();

    setHomeMenuEnable(true);
    setDim(true);
    setAPD(true);

    InputManager::getInstance().setRepeat(VPAD_BUTTON_DOWN, 300ms, 200ms);
    InputManager::getInstance().setRepeat(VPAD_BUTTON_UP, 300ms, 200ms);

    bool inMenu = true;
    while(inMenu && Utility::platformIsRunning()) { // loop until menu or app signals an exit
        if(ProcIsForeground()) { // only update in foreground
            switch(sInstance.update()) {
                case Status::CHANGED:
                    ScreenClear();
                    sInstance.drawTV();
                    sInstance.drawDRC();
                    ScreenDraw();

                    break;
                case Status::EXIT:
                    inMenu = false;
                    break;
                case Status::NONE:
                default:
                    break;
            }
        }

        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }


    setHomeMenuEnable(false);
    setDim(false);
    setAPD(false);
    
    out = OptionCB::getInternal();
    if(out.writeToFile(APP_SAVE_PATH "config.yaml") != ConfigError::NONE) {
        return Result::CONFIG_SAVE_FAILED;
    }

    if(!Utility::platformIsRunning()) {
        return Result::EXIT;
    }

    return Result::CONTINUE;
}
