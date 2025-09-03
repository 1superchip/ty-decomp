#ifndef STRUCTLIST_H
#define STRUCTLIST_H

#include "types.h"
#include "common/Heap.h"

/// @brief A fixed-capacity, reverse-fill list structure
/// @tparam T Type of elements stored in the list
template <typename T>
struct StructList {
    // Pointer to the beginning of the allocated memory block
    T* pBlock;

    // Pointer to current beginning of valid data
    T* pBegin;

    // Pointer to the end of the allocated memory block
    T* pEnd;

    void Init(int count) {
        pBlock = (T*)Heap_MemAlloc(sizeof(T) * count);
        pEnd = pBegin = &pBlock[count];
    }

    void Deinit(void) {
        Heap_MemFree((void*)pBlock);
        pBlock = pBegin = pEnd = NULL;
    }
    
    /// @brief Checks if the list contains no elements
    /// @return true if empty, false otherwise
    bool empty(void) {
        return pBegin == pEnd;
    }

    /// @brief Checks if the list has reached its capacity
    /// @return true if full, false otherwise
    bool full(void) {
        return pBegin == pBlock;
    }

    T* GetNextEntry(void) {
        if (pBlock < pBegin) {
            return --pBegin;
        }
        
        return NULL;
    }

    /// @brief Returns a pointer to the beginning of entries
    /// @return Pointer to the beginning if there is at least 1 entry, otherwise NULL
    T* GetCurrEntry(void) {
        if (pBegin != pEnd) {
            return pBegin;
        }

        return NULL;
    }

    T* GetNextEntryWithEntry(T* pCurrEntry) {
        T* pNext = pCurrEntry + 1;
        
        return pNext >= pEnd ? NULL : pNext;
    }

    void UnknownSetPointer(void) {
        pBegin = pEnd;
    }

    /// @brief Returns the number of elements currently stored in the list
    /// @return Current size
    int size(void) {
        return pEnd - pBegin;
    }

    /// @brief Returns the total number of elements that can be held
    /// @return Maximum capacity
    int capacity(void) {
        return pEnd - pBlock;
    }
    
    void Destroy(T* p) {
        if (p != pBegin) {
            *p = *pBegin;
        }

        pBegin++;
    }
};

#endif // STRUCTLIST_H
