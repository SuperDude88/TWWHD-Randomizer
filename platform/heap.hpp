#pragma once

#include <coreinit/memheap.h>

class BucketHeap {
private:
    static constexpr uint32_t FRAME_HEAP_TAG = 0x000DECAF;
    MEMHeapHandle heap = NULL;

    BucketHeap() {}
    ~BucketHeap() {}

public:
    static BucketHeap& getInstance();

    bool acquired();
    void released();

    // returns pointer in MEM0
    // the whole heap is freed automatically when losing foreground
    // this should be called again to get a new pointer when regaining foreground (in acquire callback)
    void* allocItem(const size_t& size, int64_t align = 4);

    // this can be used to free something before losing foreground if it's not needed for very long
    void freeItem(void* ptr);
};

class ForegroundHeap {
private:
    static constexpr uint32_t FRAME_HEAP_TAG = 0x000DECAF;
    MEMHeapHandle heap = NULL;

    ForegroundHeap() {}
    ~ForegroundHeap() {}

    static MEMHeapHandle& getHeap();

public:
    static ForegroundHeap& getInstance();

    bool acquired();
    void released();

    // returns pointer in MEM1
    // the whole heap is freed automatically when losing foreground
    // this should be called again to get a new pointer when regaining foreground (in acquire callback)
    void* allocItem(const size_t& size, int64_t align = 4);

    // this can be used to free something before losing foreground if it's not needed for very long
    void freeItem(void* ptr);
};
