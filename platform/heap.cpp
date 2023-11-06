#include "heap.hpp"

#include <algorithm>

#include <coreinit/memdefaultheap.h>
#include <coreinit/memfrmheap.h>
#include <coreinit/memexpheap.h>



BucketHeap& BucketHeap::getInstance() {
    static BucketHeap sInstance;

    return sInstance;
}

bool BucketHeap::acquired() {
    // set up heap
    if(!heap) {
        const MEMHeapHandle fg = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);

        const uint32_t size = MEMGetAllocatableSizeForFrmHeapEx(fg, 4);
        if(size == 0) {
            return false;
        }

        void* base = MEMAllocFromFrmHeapEx(fg, size, 4);
        if (!base) {
            return false;
        }
        
        heap = MEMCreateExpHeapEx(base, size, 0);
        if(!heap) {
            return false;
        }

        return true;
    }

    return false;
}

void BucketHeap::released() {
    // release heap
    const MEMHeapHandle fg = MEMGetBaseHeapHandle(MEM_BASE_HEAP_FG);
    
    if(heap) {
        MEMDestroyExpHeap(heap);
        heap = NULL;
    }
    
    MEMFreeToFrmHeap(fg, MEM_FRM_HEAP_FREE_ALL);
}

void* BucketHeap::allocItem(const size_t& size, int64_t align /* = 4 */) {
    align = std::max<int64_t>(4, align);

    if(!heap) {
        return nullptr;
    }

    return MEMAllocFromExpHeapEx(heap, size, align);
}

void BucketHeap::freeItem(void* ptr) {
    if(heap && ptr) {
        MEMFreeToExpHeap(heap, ptr);
    }
}



ForegroundHeap& ForegroundHeap::getInstance() {
    static ForegroundHeap sInstance;

    return sInstance;
}

bool ForegroundHeap::acquired() {
    // set up heap
    if(!heap) {
        const MEMHeapHandle mem1 = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);

        if(!MEMRecordStateForFrmHeap(mem1, FRAME_HEAP_TAG)) {
            return false;
        }

        const uint32_t size = MEMGetAllocatableSizeForFrmHeapEx(mem1, 4);
        if(size == 0) {
            return false;
        }

        void* base = MEMAllocFromFrmHeapEx(mem1, size, 4);
        if (!base) {
            return false;
        }
        
        heap = MEMCreateExpHeapEx(base, size, 0);
        if(!heap) {
            return false;
        }

        return true;
    }

    return false;
}

void ForegroundHeap::released() {
    // release heap
    const MEMHeapHandle mem1 = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM1);
    
    if(heap) {
        MEMDestroyExpHeap(heap);
        heap = NULL;
    }
    
    MEMFreeByStateToFrmHeap(mem1, FRAME_HEAP_TAG);
}

void* ForegroundHeap::allocItem(const size_t& size, int64_t align /* = 4 */) {
    align = std::max<int64_t>(4, align);

    if(!heap) {
        return nullptr;
    }

    return MEMAllocFromExpHeapEx(heap, size, align);
}

void ForegroundHeap::freeItem(void* ptr) {
    if(heap && ptr) {
        MEMFreeToExpHeap(heap, ptr);
    }
}
