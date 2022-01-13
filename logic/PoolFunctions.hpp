
#pragma once

#include <unordered_set>

// Helper function for combining two item pools
template <typename T, typename FromPool>
void AddElementsToPool(std::unordered_multiset<T>& toPool, const FromPool& fromPool) {
    for (T element : fromPool) {
        toPool.insert(element);
    }
}
