#include "pointer.hpp"

aligned_unique::~aligned_unique() {
    destroy();
}

void aligned_unique::clear() {
    buf = nullptr;
    align = std::align_val_t{1};
}

void aligned_unique::destroy() {
    if(buf) {
        operator delete(buf, align);
    }

    clear();
}

aligned_unique& aligned_unique::operator=(std::nullptr_t) {
    destroy();

    return *this;
}
aligned_unique& aligned_unique::operator=(aligned_unique&& rhs) {
    destroy();

    this->align = rhs.align;
    this->buf = rhs.buf;

    rhs.clear(); //don't deallocate buf since own it now

    return *this;
}

void* aligned_unique::get() const {
    return buf;
}
