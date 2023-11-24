#include "energy_saver.hpp"

#include <coreinit/energysaver.h>

static bool init = false;
static uint32_t dimEnabled = true;
static uint32_t apdEnabled = true;

void initEnergySaver() {
    if(!init) {
        IMIsDimEnabled(&dimEnabled);
        IMIsAPDEnabled(&apdEnabled);

        init = true;
    }

    return;
}

void setDim(const bool& enable) {
    if(enable) {
        IMEnableDim();
    }
    else {
        IMDisableDim();
    }
}

void setAPD(const bool& enable) {
    if(enable) {
        IMEnableAPD();
    }
    else {
        IMDisableAPD();
    }
}

void resetEnergySaver() {
    setDim(dimEnabled);

    if(init) {
        setAPD(apdEnabled);
    }
    else {
        IMIsAPDEnabledBySysSettings(&apdEnabled);
        setAPD(apdEnabled);
    }
}
