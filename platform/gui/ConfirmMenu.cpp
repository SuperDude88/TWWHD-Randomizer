#include "ConfirmMenu.hpp"

#include <thread>

#include <platform/input.hpp>
#include <platform/gui/screen.hpp>
#include <platform/gui/OptionActions.hpp>

using namespace std::literals::chrono_literals;

bool confirmRandomize() {
    ScreenClear();

    // pull from the internal menu config
    OSScreenPutFontEx(SCREEN_TV, 0, 0, ("Preparing to patch with seed \"" + getSeed() + "\" and hash \"" + getSeedHash() + "\".").c_str());
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Look at the TV for more information.");

    OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press A to continue with randomization.\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press A to continue with randomization.\n");
    OSScreenPutFontEx(SCREEN_TV, 0, 3, "Press B to go back.\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 3, "Press B to go back.\n");

    ScreenDraw();
    

    while(true) {
        if(InputManager::getInstance().poll() != InputError::NONE) {
            continue;
        }

        if(InputManager::getInstance().pressed(VPAD_BUTTON_B)) { // B gets priority if both are pressed
            return false;
        }
        if(InputManager::getInstance().pressed(VPAD_BUTTON_A)) {
            return true;
        }
        
        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }
}
