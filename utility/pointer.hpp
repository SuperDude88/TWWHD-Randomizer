#pragma once

#include <cstddef>
#include <new>

class aligned_unique {
private:
    std::align_val_t align;
    void* buf;

    void clear(); //clear current pointer but don't deallocate
    void destroy(); //deallocate and clear
public:
    constexpr aligned_unique() :
        aligned_unique(nullptr)
    {}
    constexpr aligned_unique(std::nullptr_t) {
        buf = nullptr;
        align = std::align_val_t{1};
    }
    aligned_unique(const size_t& size_, const std::align_val_t& align_) :
        align(align_),
        buf(operator new(size_, align_))
    {}
    aligned_unique(aligned_unique&& other) = default;
    aligned_unique(const aligned_unique&) = delete;

    ~aligned_unique();

    aligned_unique& operator=(std::nullptr_t);
    aligned_unique& operator=(aligned_unique&& rhs);
    aligned_unique& operator=(const aligned_unique&) = delete;

    void* get() const;
};

inline aligned_unique make_aligned_unique(const size_t& size, const size_t& align) { return aligned_unique(size, std::align_val_t{align}); }