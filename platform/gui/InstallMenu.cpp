#include "InstallMenu.hpp"

#include <thread>

#include <vpad/input.h>

#include <platform/gui/screen.hpp>

using namespace std::literals::chrono_literals;

MCPInstallTarget pickInstallLocation() {
    ScreenClear();

    OSScreenPutFontEx(SCREEN_TV, 0, 0, "Output channel does not currently exist.\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Output channel does not currently exist.\n");

    OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press X to install it to console storage\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press X to install it to console storage\n");
    OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press Y to install it to USB\n");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press Y to install it to USB\n");

    ScreenDraw();
    
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
