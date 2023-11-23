#include "home.hpp"

#include <coreinit/systeminfo.h>

static bool homeEnabled = true;

void initHomeMenu() {
    homeEnabled = OSIsHomeButtonMenuEnabled();
}

void setHomeMenuEnable(const bool& enable) {
    OSEnableHomeButtonMenu(enable);
}

void resetHomeMenu() {
    OSEnableHomeButtonMenu(homeEnabled);
}
