#ifndef COMMON_PTRLISTDL
#define COMMON_PTRLISTDL

#include "types.h"
#include "common/Heap.h"

extern int gEmptyPtrListDL[2]; // Utils.cpp


// I have no idea if this is closer than before
// This does seem closer because the code doesn't need as many casts

template <typename T>
inline void Swap(T& p, T& p1) {
    T tmp = p;
    p = p1;
    p1 = tmp;
}

template <typename T>
struct PtrListDL {
    T **pMem;

    void Init(int count, int size);
    void Deinit(void);
    void Destroy(T** p);
    void Destroy(T* p);

    T** GetMem(void) {
        return pMem;
    }

    void* GetEnd(void) {
        T** ptr = pMem;
        while (*ptr != NULL) {
            ptr++;
        }
        return (void*)ptr;
    }

    inline int GetSize(void) {
        return ((int)GetEnd() - (int)pMem) / 4;
    }

    inline bool IsFull(void) {
        return pMem[-1] == NULL;
    }
    
    inline T* GetNextEntry(void) {
        return *--pMem;
    }
    
    inline void Reset(void) {
        while (*pMem != NULL) {
            pMem++;
        }
    }

    inline bool IsEmpty(void) {
        return *pMem == NULL;
    }
};

// defining this in the struct with "-inline auto" does inline this function but defining it here doesn't auto inline
// need the inline def here
template <typename T>
inline void PtrListDL<T>::Init(int count, int size) {
    if (count == 0) {
        pMem = (T**)&gEmptyPtrListDL[1];
        return;
    }
    // count * size = structure array size
    // count * 4 = pointer array size
    pMem = (T**)Heap_MemAlloc(count * size + (count + 2) * sizeof(T*));
    T* memory = (T*)pMem;
    /*pMem = (T**)((int)pMem + count * size);
    *pMem = NULL;*/
    *(pMem = (T**)((int)pMem + count * size)) = NULL; // set mem pointer to end of array
    for (int i = 0; i < count; i++) {
        *++pMem = memory;
        memory = ((T*)memory + 1);
    }
    
    *++pMem = NULL;
}

template <typename T>
inline void PtrListDL<T>::Deinit(void) {
    T* memory = (T*)pMem;
    T** temp = pMem;
    while (*--temp != NULL) {
        if (*temp < memory) {
            memory = *temp;
        }
    }

    if (temp != (T**)&gEmptyPtrListDL[0]) {
        T** ptrs = pMem;
        while (*ptrs != NULL) {
            if (*ptrs < memory) {
                memory = *ptrs;
            }
            ptrs++;
        }

        Heap_MemFree(memory);
    }

    pMem = NULL;
}

template <typename T>
inline void PtrListDL<T>::Destroy(T** p) {
    Swap<T*>(*pMem++, *p);
}

template <typename T>
inline void PtrListDL<T>::Destroy(T* p) {
    T** memPtr = pMem;
    while (*memPtr != NULL) {
        if (*memPtr == p) {
            Destroy(memPtr);
            break;
        }

        memPtr++;
    }
}

#endif // COMMON_PTRLISTDL
