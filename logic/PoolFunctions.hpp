
#pragma once

#include <algorithm>
#include <vector>
#include <iostream>

template <typename T, typename Predicate>
static void erase_if(std::vector<T>& vector, Predicate pred) {
  vector.erase(std::remove_if(begin(vector), end(vector), pred), end(vector));
}

template <typename T, typename Predicate>
std::vector<T> filterFromPool(std::vector<T>& vector, Predicate pred, bool eraseAfterFilter = false) {
  std::vector<T> filteredPool = {};
  std::copy_if(vector.begin(), vector.end(), std::back_inserter(filteredPool), pred);

  if (eraseAfterFilter) {
    erase_if(vector, pred);
  }

  return filteredPool;
}

template <typename T, typename Predicate>
std::vector<T> filterAndEraseFromPool(std::vector<T>& vector, Predicate pred) {
  return filterFromPool(vector, pred, true);
}

// Helper function for combining two item pools
template <typename T, typename FromPool>
void addElementsToPool(std::vector<T>& toPool, const FromPool& fromPool)
{
    toPool.insert(toPool.end(), fromPool.begin(), fromPool.end());
}

template <typename T, typename Container>
bool elementInPool(T& element, const Container& container)
{
    return std::find(container.begin(), container.end(), element) != container.end();
}

template <typename T, typename Container>
size_t elementCountInPool(T& element, const Container& container)
{
    return std::count(container.begin(), container.end(), element);
}
