#ifndef STRUCTLIST_H
#define STRUCTLIST_H

#include "types.h"

template <typename T>
struct StructList {
    T* pStart; // pBlock?
    T* pCurr;
    T* pEnd;

    void Init(int count) {
        pStart = (T*)Heap_MemAlloc(sizeof(T) * count);
        pCurr = &pStart[count];
        pEnd = &pStart[count];
    }

    void Deinit(void) {
        Heap_MemFree((void*)pStart);
        pStart = pCurr = pEnd = NULL;
    }

    bool CheckMemory(void) {
        return pCurr == pStart;
    }
    
    bool CheckMemory2(void) {
        return pCurr == pEnd;
    }

    T* GetNextEntry(void) {
        if (pStart < pCurr) {
            return --pCurr;
        }
        
        return NULL;
    }

    T* GetCurrEntry(void) {
        if (pCurr != pEnd) {
            return pCurr;
        }

        return NULL;
    }

    T* GetNextEntryWithEntry(T* pCurrEntry) {
        T* pNext = pCurrEntry + 1;
        
        return pNext >= pEnd ? NULL : pNext;
    }

    void UnknownSetPointer(void) {
        pCurr = pEnd;
    }

    int GetCount(void) {
        return pEnd - pCurr;
    }
    
    void CopyEntry(T* p) {
        if (p != pCurr) {
            *p = *pCurr;
        }

        pCurr++;
    }
};

#endif // STRUCTLIST_H
