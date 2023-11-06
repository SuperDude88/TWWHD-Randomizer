#include "proc.hpp"

#include <platform/heap.hpp>

#include <coreinit/core.h>
#include <coreinit/foreground.h>
#include <sysapp/launch.h>

static bool running = false;
static bool foreground = true;

std::multiset<CombinedCB> callbacks = {};

static uint32_t saveCB(void* arg) {
    OSSavesDone_ReadyToRelease();

    return 0;
}

static uint32_t acquireCB(void* arg = nullptr) {
    BucketHeap::getInstance().acquired();
    ForegroundHeap::getInstance().acquired();
    for(const CombinedCB& cb : callbacks) {
        (*cb.acquired)(cb.arg); //TODO: error handling, return values?
    }

    foreground = true;

    return 0;
}

static uint32_t releaseCB(void* arg = nullptr) {
    for(auto it = callbacks.rbegin(); it != callbacks.rend(); it++ ) {
        (*it->released)(it->arg); //TODO: error handling, return values?
    }
    ForegroundHeap::getInstance().released();
    BucketHeap::getInstance().released();

    foreground = false;

    return 0;
}

static void ProcSetCallback(ProcUICallbackType type, ProcUICallback cb, void* arg = nullptr, uint32_t priority = 100) {
    ProcUIRegisterCallback(type, cb, arg, priority);
}

void ProcInit() {
    running = true;

    ProcUIInitEx(&saveCB, nullptr);
    ProcSetCallback(PROCUI_CALLBACK_ACQUIRE, &acquireCB);
    ProcSetCallback(PROCUI_CALLBACK_RELEASE, &releaseCB);

    acquireCB(); // already in foreground, so call this to set up bucket/etc
}

void ProcExit() {
    // Tell the console we want to leave
    SYSLaunchMenu();
}

static void ProcStop() {
    running = false;
    ProcUIShutdown();
}

bool ProcIsRunning(ProcUIStatus* outStat /* = nullptr */) {
    if(!running) return running;

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

    if(outStat) {
        *outStat = status;
    }
    
    return running;
}

bool ProcIsForeground() {
    return foreground;
}

ScopedCallback::ScopedCallback(const CombinedCB& cb) :
    it(callbacks.insert(cb))
{}

ScopedCallback::~ScopedCallback() {
    callbacks.erase(it);
}

void addCallbacks(const CombinedCB& cb) {
    callbacks.insert(cb);
}
