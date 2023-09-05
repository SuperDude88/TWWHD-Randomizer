#include "SeedMenu.hpp"

#include <thread>

#include <vpad/input.h>

#include <seedgen/seed.hpp>
#include <platform/gui/screen.hpp>

using namespace std::literals::chrono_literals;

bool pickSeed(std::string& current) {
    bool seedChanged = false;
    while(true) {
        ScreenClear();

        OSScreenPutFontEx(SCREEN_TV, 0, 0, ("The current seed is \"" + current + "\".\n").c_str());
        OSScreenPutFontEx(SCREEN_DRC, 0, 0, ("The current seed is \"" + current + "\".\n").c_str());

        OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press A to continue with this seed\n");
        OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press A to continue with this seed\n");
        OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press Y to generate a new seed\n");
        OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press Y to generate a new seed\n");

        ScreenDraw();

        while(true) {
            VPADStatus status{};
            VPADRead(VPAD_CHAN_0, &status, 1, nullptr);

            if (status.trigger & VPAD_BUTTON_A) { //A gets priority if both are pressed
                return seedChanged; //finish picking a seed, leave the whole function
            }
            if (status.trigger & VPAD_BUTTON_Y) {
                current = generate_seed();
                seedChanged = true;
                break; //break the input loop, confirm with new seed
            }

            std::this_thread::sleep_for(17ms); //update ~60 times a second
        }
    }
}

bool exitForConfig() {
    ScreenClear();

    OSScreenPutFontEx(SCREEN_TV, 0, 0, "A config file is available\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "A config file is available\n");

    OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press A to use this config\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press A to use this config\n");
    OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press B to exit and modify the config\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press B to exit and modify the config\n");

    ScreenDraw();
    
    for(size_t i = 0; i < 3600; i++) { //time out after ~1 minute
        VPADStatus status{};
        VPADRead(VPAD_CHAN_0, &status, 1, nullptr);

        if (status.trigger & VPAD_BUTTON_B) { //B gets priority if both are pressed
            return true;
        }
        if (status.trigger & VPAD_BUTTON_A) {
            return false;
        }
        
        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }

    return true;
}
