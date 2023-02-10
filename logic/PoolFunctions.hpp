
#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

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
template <typename FromPool, typename Container>
void addElementsToPool(Container& toPool, const FromPool& fromPool)
{
    toPool.insert(toPool.end(), fromPool.begin(), fromPool.end());
}

template <typename T>
void addElementToPool(std::vector<T>& toPool, T element, int numberToAdd = 1)
{
    for (int i = 0; i < numberToAdd; i++)
    {
        toPool.push_back(element);
    }
}

template <typename T, typename Container>
bool elementInPool(T element, const Container& container)
{
    return std::find(container.begin(), container.end(), element) != container.end();
}

template <typename T, typename Container>
size_t elementCountInPool(T& element, const Container& container)
{
    return std::count(container.begin(), container.end(), element);
}

template <typename T, typename Container>
T removeElementFromPool( Container& container, T element, int numberToRemove = 1)
{
    // Return the same element if we don't find any in the container
    T retElement = element;
    for (int i = 0; i < numberToRemove; i++)
    {
        auto itr = std::find(container.begin(), container.end(), element);
        if (itr != container.end())
        {
            retElement = *itr;
            container.erase(itr);
        }
    }
    return retElement;
}

template <typename Container>
void removeElementsFromPool( Container& container, Container& elementsToRemove)
{
    for (auto element : elementsToRemove)
    {
        removeElementFromPool(container, element);
    }
}

template <typename First, typename... T>
bool isAnyOf(First&& first, T&&... t)
{
    return ((first == t) || ...);
}

template <typename First, typename... T>
bool isNoneOf(First&& first, T&&... t)
{
    return !((first == t) || ...);
}
