#include "controller.hpp"

#include <thread>

#include <vpad/input.h>

#include <utility/platform.hpp>



bool exitForConfig() {
    Utility::platformLog("A config file is available.\n");
    Utility::platformLog("Press A to use this config\n");
    Utility::platformLog("Press B to exit and modify the config\n");
    
    for(size_t i = 0; i < 3600; i++) { //time out after ~1 minute
        VPADStatus status{};
        VPADRead(VPAD_CHAN_0, &status, 1, nullptr);

        if (status.trigger & VPAD_BUTTON_B) { //B gets priority if both are pressed
            return true;
        }
        if (status.trigger & VPAD_BUTTON_A) {
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(17)); //update ~60 times a second
    }

    return true;
}
