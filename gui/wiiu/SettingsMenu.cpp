#include "SettingsMenu.hpp"

#include <thread>
#include <chrono>

#include <version.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/path.hpp>
#include <platform/proc.hpp>
#include <platform/home.hpp>
#include <platform/energy_saver.hpp>
#include <platform/input.hpp>
#include <gui/wiiu/screen.hpp>
#include <gui/wiiu/ConfirmMenu.hpp>

static const std::string line_break(ScreenSizeData::tv_line_length, '=');

static void showVersionWarning() {
    ScreenClear();

    OSScreenPutFontEx(SCREEN_TV, 0, 0, "Could not determine Randomizer version. Please tell a dev if you see this message.");
    OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press any button to continue.");
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Could not determine Randomizer version. Please tell a dev if you see this message.");
    OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press any button to continue.");

    ScreenDraw();
    
    while(true) {
        if(InputManager::getInstance().poll() != InputError::NONE) {
            continue;
        }

        if(InputManager::getInstance().anyButtonPressed()) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(17)); //update ~60 times a second
    }
}

SettingsMenu::SettingsMenu() {
    pages[0] = std::make_unique<SeedPage>();
    pages[1] = std::make_unique<ProgressionPage>();
    pages[2] = std::make_unique<HintsPage>();
    pages[3] = std::make_unique<EntrancePage>();
    pages[4] = std::make_unique<ItemsPage>(); 
    pages[5] = std::make_unique<LocationsPage>(); 
    pages[6] = std::make_unique<ConveniencePage>();
    pages[7] = std::make_unique<AdvancedPage>();
    pages[8] = std::make_unique<ColorPage>();
    pages[9] = std::make_unique<MetaPage>();
}


SettingsMenu& SettingsMenu::getInstance() {
    static SettingsMenu sInstance;
    return sInstance;
}

SettingsMenu::Status SettingsMenu::update() {
    if(InputManager::getInstance().poll() != InputError::NONE) {
        return Status::NONE;
    }

    if(InputManager::getInstance().pressed(ButtonInfo::PLUS)) {
        pages[curPage]->close();

        return Status::EXIT;
    }

    bool moved = false;
    if(InputManager::getInstance().pressed(ButtonInfo::ZL)) {
        pages[curPage]->close();

        if(curPage <= 0) {
            curPage = pages.size() - 1; //wrap on leftmost page
        }
        else {
            curPage -= 1; //left one page
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::ZR)) {
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
    std::string headerFirstLine;
    // 
    for(size_t i = 0; i < 7; i++) {
        if(i == curPage) {
            headerFirstLine += '<' + pages[i]->getName() + '>';
        }
        else {
            headerFirstLine += ' ' + pages[i]->getName() + ' ';
        }
        headerFirstLine += '|';
    }

    std::string headerSecondLine;
    for(size_t i = 7; i < pages.size(); i++) {
        if(i == curPage) {
            headerSecondLine += '<' + pages[i]->getName() + '>';
        }
        else {
            headerSecondLine += ' ' + pages[i]->getName() + ' ';
        }

        //not the last item
        if(i + 1 != pages.size()) {
            headerSecondLine += '|';
        }
    }

    OSScreenPutFontEx(SCREEN_TV, 0, 0, headerFirstLine.c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, 1, headerSecondLine.c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, 2, line_break.c_str());
    
    pages[curPage]->drawTV();

    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 3, line_break.c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 2, "ZL Left Page | A Toggle Option | DPad Navigate | ZR Right Page | Start (+) to Randomize");
    OSScreenPutFontEx(SCREEN_TV, 0, ScreenSizeData::tv_num_lines - 1, "TWWHD Randomizer Version " RANDOMIZER_VERSION);
}

void SettingsMenu::drawDRC() const {
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Look at the TV for the patcher menu.");

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

SettingsMenu::Result SettingsMenu::run() {
    using namespace std::literals::chrono_literals;

    SettingsMenu& sInstance = getInstance();

    if (std::string(RANDOMIZER_VERSION).empty()) {
        showVersionWarning();
    }

    if(const ConfigError err = OptionCB::loadConfig(); err != ConfigError::NONE) {
        ErrorLog::getInstance().log("Failed to prepare config, ERROR: " + ConfigErrorGetName(err));
        LOG_ERR_AND_RETURN(Result::CONFIG_ERROR);
    }

    // void* arg is in the place of "this" pointer
    ScopedCallback procCB({2, &SettingsMenu::acquireCB, &SettingsMenu::releaseCB});
    acquireCB();

    sInstance.pages[sInstance.curPage]->open();

    setHomeMenuEnable(true);
    setDim(true);
    setAPD(true);

    InputManager::getInstance().setRepeat(ButtonInfo::DOWN, 300ms, 200ms);
    InputManager::getInstance().setRepeat(ButtonInfo::UP, 300ms, 200ms);

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
                    if(confirmRandomize()) {
                        inMenu = false;
                    }
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

    if(OptionCB::getInternal().writeToFile(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml") != ConfigError::NONE) {
        LOG_ERR_AND_RETURN(Result::CONFIG_ERROR);
    }

    if(!Utility::platformIsRunning()) {
        return Result::EXIT;
    }

    return Result::CONTINUE;
}
