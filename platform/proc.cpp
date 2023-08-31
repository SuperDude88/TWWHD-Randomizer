#include "proc.hpp"

#include <coreinit/core.h>
#include <coreinit/foreground.h>
#include <sysapp/launch.h>

static bool running = false;

static uint32_t saveCB(void* arg) {
    OSSavesDone_ReadyToRelease();

    return 0;
}

void ProcInit() {
    running = true;

    ProcUIInitEx(&saveCB, nullptr);
}

void ProcExit() {
    // Tell the console we want to leave
    SYSLaunchMenu();
}

static void ProcStop() {
    running = false;
    ProcUIShutdown();
}

bool ProcIsRunning() {
    ProcUIStatus status;
    if(OSIsMainCore()) {
        status = ProcUIProcessMessages(true);
    }
    else{
        status = ProcUISubProcessMessages(true);
    }

    switch(status) {
        // Can use all the console resources
        case PROCUI_STATUS_IN_FOREGROUND:
            break;
        // Can only use limited resources
        case PROCUI_STATUS_IN_BACKGROUND:
            break;
        // Must release foreground
        case PROCUI_STATUS_RELEASE_FOREGROUND:
            ProcUIDrawDoneRelease();
            break;
        // Must release resources and exit
        case PROCUI_STATUS_EXITING:
            ProcStop();
            break;
    }
    
    return running;
}

void ProcSetCallback(ProcUICallbackType type, ProcUICallback cb, void* arg /* = nullptr */, uint32_t priority /* = 100 */) {
    ProcUIRegisterCallback(type, cb, arg, priority);
}
