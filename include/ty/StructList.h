#ifndef STRUCTLIST_H
#define STRUCTLIST_H

template <typename T>
struct StructList {
    T* pStart;
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
};

#endif // STRUCTLIST_H
