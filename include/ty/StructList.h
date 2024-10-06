#ifndef STRUCTLIST_H
#define STRUCTLIST_H

#include "types.h"

template <typename T>
struct StructList {
    T* pStart; // pBlock?
    T* pEnd;
    T* pEnd2;

    void Init(int count) {
        pStart = (T*)Heap_MemAlloc(sizeof(T) * count);
        pEnd = &pStart[count];
        pEnd2 = &pStart[count];
    }
    void Deinit(void) {
        Heap_MemFree((void*)pStart);
        pStart = pEnd = pEnd2 = NULL;
    }
    bool CheckMemory(void) {
        return pEnd == pStart;
    }
    bool CheckMemory2(void) {
        return pEnd == pEnd2;
    }
    T* GetNextEntry(void) {
        if (pStart < pEnd) {
            return --pEnd;
        }
        return NULL;
    }
    T* GetCurrEntry(void) {
        if (pEnd != pEnd2) {
            return pEnd;
        }
        return NULL;
    }
    T* GetNextEntryWithEntry(T* pCurrEntry) {
        T* pNext = pCurrEntry + 1;
        if (pNext >= pEnd2) {
            pNext = NULL;
        }
        return pNext;
    }
    void UnknownSetPointer(void) {
        pEnd = pEnd2;
    }
    int GetCount(void) {
        return (pEnd2 - pEnd);
    }
    void CopyEntry(T* p) {
        if (p != pEnd) {
            *p = *pEnd;
        }
        pEnd++;
    }
};

#endif // STRUCTLIST_H
