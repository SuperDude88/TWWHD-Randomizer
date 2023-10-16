#include "SettingsMenu.hpp"

#include <thread>
#include <chrono>

#include <version.hpp>
#include <utility/platform.hpp>
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
}

SettingsMenu::Status SettingsMenu::update() {
    VPADStatus stat{};
    VPADRead(VPAD_CHAN_0, &stat, 1, nullptr);

    if(stat.trigger & VPAD_BUTTON_PLUS) {
        return Status::EXIT;
    }

    bool moved = false;
    if(stat.trigger & VPAD_BUTTON_ZL) {
        if(curPage <= 0) {
            curPage = pages.size() - 1; //wrap on leftmost page
        }
        else {
            curPage -= 1; //left one page
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_ZR) {
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
    
    return (moved || pages[curPage]->update(stat)) ? Status::CHANGED : Status::NONE;
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

bool SettingsMenu::run(Config& out) {
    using namespace std::literals::chrono_literals;

    static SettingsMenu sInstance;

    OptionCB::setInternal(out);
    
    ScreenClear();
    sInstance.drawTV();
    sInstance.drawDRC();
    ScreenDraw();

    bool inMenu = true;
    while(inMenu && Utility::platformIsRunning()) {
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

        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }
    
    out = OptionCB::getInternal();
    if(out.writeToFile(APP_SAVE_PATH "config.yaml") != ConfigError::NONE) {
        return true;
    }

    if(!Utility::platformIsRunning()) {
        return true;
    }

    return false;
}
