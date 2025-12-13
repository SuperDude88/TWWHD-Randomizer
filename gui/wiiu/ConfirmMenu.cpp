#include "ConfirmMenu.hpp"

#include <thread>

#include <platform/input.hpp>
#include <gui/wiiu/screen.hpp>
#include <gui/wiiu/OptionActions.hpp>

using namespace std::literals::chrono_literals;

bool confirmRandomize() {
    ScreenClear();

    // pull from the internal menu config
    OSScreenPutFontEx(SCREEN_TV, 0, 0, ("Preparing to patch with seed \"" + getSeed() + "\" and hash \"" + getSeedHash() + "\".").c_str());
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Look at the TV for more information.");

    OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press A to continue with randomization.");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press A to continue with randomization.");
    OSScreenPutFontEx(SCREEN_TV, 0, 3, "Press B to go back.");
    OSScreenPutFontEx(SCREEN_DRC, 0, 3, "Press B to go back.");

    ScreenDraw();
    

    while(true) {
        if(InputManager::getInstance().poll() != InputError::NONE) {
            continue;
        }

        if(InputManager::getInstance().pressed(ButtonInfo::B)) { // B gets priority if both are pressed
            return false;
        }
        if(InputManager::getInstance().pressed(ButtonInfo::A)) {
            return true;
        }
        
        std::this_thread::sleep_for(17ms); // update ~60 times a second
    }
}
