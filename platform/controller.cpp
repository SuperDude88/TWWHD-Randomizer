#include "controller.hpp"

#include <thread>

#include <vpad/input.h>

#include <utility/platform.hpp>

using namespace std::literals::chrono_literals;


bool exitForConfig() {
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
        
        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }

    return true;
}

MCPInstallTarget pickInstallLocation() {
    Utility::platformLog("Press X to install it to console storage\n");
    Utility::platformLog("Press Y to install it to USB\n");
    
    while(true) { //not sure if this should have a timeout
        VPADStatus status{};
        VPADRead(VPAD_CHAN_0, &status, 1, nullptr);

        if (status.trigger & VPAD_BUTTON_Y) { // Y gets priority if both are pressed
            return MCPInstallTarget::MCP_INSTALL_TARGET_USB;
        }
        if (status.trigger & VPAD_BUTTON_X) {
            return MCPInstallTarget::MCP_INSTALL_TARGET_MLC;
        }
        
        std::this_thread::sleep_for(17ms); //update ~60 times a second
    }
}
