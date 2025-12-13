#include "InstallMenu.hpp"

#include <thread>

#include <platform/input.hpp>
#include <gui/wiiu/screen.hpp>

using namespace std::literals::chrono_literals;

MCPInstallTarget pickInstallLocation() {
    ScreenClear();

    OSScreenPutFontEx(SCREEN_TV, 0, 0, "Output channel does not currently exist");
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Output channel does not currently exist.");

    OSScreenPutFontEx(SCREEN_TV, 0, 1, "Press X to install it to console storage");
    OSScreenPutFontEx(SCREEN_DRC, 0, 1, "Press X to install it to console storage");
    OSScreenPutFontEx(SCREEN_TV, 0, 2, "Press Y to install it to USB (recommended)");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "Press Y to install it to USB (recommended)");

    ScreenDraw();
    
    while(true) { // not sure if this should have a timeout
        if(InputManager::getInstance().poll() != InputError::NONE) {
            continue;
        }

        if(InputManager::getInstance().pressed(ButtonInfo::Y)) { // Y gets priority if both are pressed
            return MCPInstallTarget::MCP_INSTALL_TARGET_USB;
        }
        if(InputManager::getInstance().pressed(ButtonInfo::X)) {
            return MCPInstallTarget::MCP_INSTALL_TARGET_MLC;
        }
        
        std::this_thread::sleep_for(17ms); // update ~60 times a second
    }
}
