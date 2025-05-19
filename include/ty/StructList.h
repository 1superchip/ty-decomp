#ifndef STRUCTLIST_H
#define STRUCTLIST_H

#include "types.h"

template <typename T>
struct StructList {
    T* pBlock;
    T* pBegin;
    T* pEnd;

    void Init(int count) {
        pBlock = (T*)Heap_MemAlloc(sizeof(T) * count);
        pBegin = &pBlock[count];
        pEnd = &pBlock[count];
    }

    void Deinit(void) {
        Heap_MemFree((void*)pBlock);
        pBlock = pBegin = pEnd = NULL;
    }

    bool CheckMemory(void) {
        return pBegin == pBlock;
    }
    
    bool CheckMemory2(void) {
        return pBegin == pEnd;
    }

    T* GetNextEntry(void) {
        if (pBlock < pBegin) {
            return --pBegin;
        }
        
        return NULL;
    }

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

    int GetCount(void) {
        return pEnd - pBegin;
    }
    
    // Destroy?
    void CopyEntry(T* p) {
        if (p != pBegin) {
            *p = *pBegin;
        }

        pBegin++;
    }
};

#endif // STRUCTLIST_H
