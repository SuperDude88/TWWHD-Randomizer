#include "screen.hpp"

#include <coreinit/cache.h>
#include <platform/proc.hpp>
#include <utility/pointer.hpp>

static size_t tvBufSize;
static size_t drcBufSize;
static aligned_unique tvBuf;
static aligned_unique drcBuf;

static uint32_t sBackgroundColor = 0x993333FF;

static uint32_t SetScreenBuffers(void* arg = nullptr) {
    OSScreenSetBufferEx(SCREEN_TV, tvBuf.get());
    OSScreenSetBufferEx(SCREEN_DRC, drcBuf.get());

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    
    return 0;
}

void ConsoleScreenInit() {
    OSScreenInit();
    
    tvBufSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    drcBufSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    tvBuf = make_aligned_unique(tvBufSize, 0x100);
    drcBuf = make_aligned_unique(drcBufSize, 0x100);
    
    SetScreenBuffers();
    ProcSetCallback(PROCUI_CALLBACK_ACQUIRE, SetScreenBuffers);

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
    DCFlushRange(tvBuf.get(), tvBufSize);
    DCFlushRange(drcBuf.get(), drcBufSize);
    
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}
