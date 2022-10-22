#pragma once

#include <cstdint>

template<typename T>
T roundUp(const T& val, const T& multiple) {
    if(val % multiple == 0) return val;
    return val + multiple - (val % multiple);
}