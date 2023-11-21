#include "screen.hpp"

#include <platform/proc.hpp>
#include <platform/heap.hpp>

#include <coreinit/cache.h>

static size_t tvBufSize;
static size_t drcBufSize;
static void* tvBuf;
static void* drcBuf;

static uint32_t sBackgroundColor = 0x993333FF;

static uint32_t OSScreenAcquired(void* arg = nullptr) {
    tvBuf = BucketHeap::getInstance().allocItem(tvBufSize, 0x100);
    drcBuf = BucketHeap::getInstance().allocItem(drcBufSize, 0x100);
    OSScreenSetBufferEx(SCREEN_TV, tvBuf);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuf);

    return 0;
}

static uint32_t OSScreenReleased(void* arg = nullptr) {
    // MEM0 is freed by BucketHeap

    return 0;
}

void ConsoleScreenInit() {
    OSScreenInit();
    
    tvBufSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    drcBufSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    addCallbacks({1, &OSScreenAcquired, &OSScreenReleased});

    OSScreenAcquired();

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);
}

void SetScreenColor(const uint32_t& col) {
    sBackgroundColor = col;
}

void ScreenClear() {
    OSScreenClearBufferEx(SCREEN_TV, sBackgroundColor);
    OSScreenClearBufferEx(SCREEN_DRC, sBackgroundColor);
}

void ScreenDraw() {
    DCFlushRange(tvBuf, tvBufSize);
    DCFlushRange(drcBuf, drcBufSize);
    
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}
