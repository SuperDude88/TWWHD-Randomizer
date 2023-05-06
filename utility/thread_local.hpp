#pragma once

#include <thread>
#ifdef DEVKITPRO
#include <cstdint>
#include <list>
#include <coreinit/thread.h>
#endif

#ifdef DEVKITPRO
enum struct DataIDs : uint32_t {
    FILE_OP_BUFFER = 0
};
#endif

template<typename T>
class ThreadLocal {
#ifdef DEVKITPRO
    std::list<T> data;
    const uint32_t id;

public:
    ThreadLocal(const DataIDs id_) : id(static_cast<std::underlying_type_t<DataIDs>>(id_)) {}
    ~ThreadLocal() {
        //TODO: unregister the data on all the threads somehow?
    }

    operator T&() {
        if(OSGetThreadSpecific(id) == nullptr) {
            data.emplace_back();
            OSSetThreadSpecific(id, &data.back());
        }
        return *reinterpret_cast<T*>(OSGetThreadSpecific(id));
    }
#else
    inline static thread_local T data;

public:
    ThreadLocal() {}

    operator T&() {
        return data;
    }
#endif
};
