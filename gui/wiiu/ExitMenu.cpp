#include "ExitMenu.hpp"

#include <thread>

#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <platform/input.hpp>
#include <gui/wiiu/screen.hpp>

using namespace std::literals::chrono_literals;

void waitForExitConfirm(const ExitMode& mode) {
    if(mode == ExitMode::IMMEDIATE) {
        return;
    }

    ScreenClear();

    switch(mode) {
        case ExitMode::PLATFORM_ERROR:
            OSScreenPutFontEx(SCREEN_TV, 0, 0, "Failed to initialize platform, check the error log for details.");
            OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Failed to initialize platform, check the error log for details.");
            OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press any button to exit.");
            OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press any button to exit.");

            break;
        case ExitMode::GUI_ERROR:
            OSScreenPutFontEx(SCREEN_TV, 0, 0, ("GUI encountered an error, check the error log for details.\n" + ErrorLog::getInstance().getLastErrors() + "\nPress any button to exit.").c_str());
            OSScreenPutFontEx(SCREEN_DRC, 0, 0, ("GUI encountered an error, check the error log for details.\n" + ErrorLog::getInstance().getLastErrors() + "\nPress any button to exit.").c_str());

            break;
        case ExitMode::RANDOMIZATION_COMPLETE:
            OSScreenPutFontEx(SCREEN_TV, 0, 0, "Randomization complete! Press any button to exit.");
            OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Randomization complete! Press any button to exit.");

            break;
        case ExitMode::RANDOMIZATION_ERROR:
            OSScreenPutFontEx(SCREEN_TV, 0, 0, ("An error has occured! See the error log for details.\n" + ErrorLog::getInstance().getLastErrors() + "\nPress any button to exit.").c_str());
            OSScreenPutFontEx(SCREEN_DRC, 0, 0, ("An error has occured! See the error log for details.\n" + ErrorLog::getInstance().getLastErrors() + "\nPress any button to exit.").c_str());

            break;
    }
    
    ScreenDraw();

    while(Utility::platformIsRunning()) {
        if(InputManager::getInstance().poll() != InputError::NONE) {
            continue;
        }
        
        if(InputManager::getInstance().anyButtonPressed()) {
            return;
        }
        
        std::this_thread::sleep_for(17ms); // update ~60 times a second
    }

    return;
}
