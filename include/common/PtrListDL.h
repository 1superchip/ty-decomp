#ifndef COMMON_PTRLISTDL
#define COMMON_PTRLISTDL

#include "types.h"
#include "common/Heap.h"

extern int gEmptyPtrListDL[2]; // Utils.cpp

template <typename T>
inline void Swap(T& p, T& p1) {
    T tmp = p;
    p = p1;
    p1 = tmp;
}

template <typename T>
struct PtrListDL {
	T *pMem;
	void Init(int, int);
	void Deinit(void);
	void Destroy(T** p);
	void Destroy(T* p);
	void* GetEnd(void) {
        void* ptr = pMem;
        while ((int*)(*(int*)ptr) != 0) {
			((int*)ptr)++;
        }
        return ptr;
    }
	inline int GetSize(void) {
        return ((int)GetEnd() - (int)pMem) / 4;
    }
	inline bool IsFull(void) {
        return (int*)*((int*)pMem - 1) == 0;
    }
	inline T* GetNextEntry(void) {
        return *--(T**)pMem;
    }
};

template <typename T>
inline void PtrListDL<T>::Init(int count, int size) {
    // count * size = structure array size
    // count * 4 = pointer array size
    pMem = (T*)Heap_MemAlloc(count * size + (count + 2) * 4);
    T* memory = pMem;
    int* arrayEnd = (int*)((int)memory + count * size); // go to array memory end
    *(int*)(pMem = (T*)arrayEnd) = 0; // set mem pointer to end of array
    int i = 0;
    while(i < count) {
        int* pNext = (int*)pMem + 1;
        pMem = (T*)pNext;
        *pNext = (int)memory;
        memory = ((T*)memory + 1);
        i++;
    }
    *(++(int*)pMem) = NULL;
}

template <typename T>
inline void PtrListDL<T>::Deinit(void) {
    T** ptrs = (T**)pMem;
    T* memory = pMem;
    T* temp = memory;
    while ((int*)*--(int*)temp != NULL) {
        if (*(T**)temp < memory) {
            memory = *(T**)temp;
        }
    }
    if (temp != (T*)&gEmptyPtrListDL[0]) {
        while(*(T**)ptrs != NULL) {
            if (*(T**)ptrs < (T*)memory) {
                memory = *(T**)ptrs;
            }
            ((T**)ptrs)++;
        }
        Heap_MemFree(memory);
    }
    pMem = NULL;
}

template <typename T>
inline void PtrListDL<T>::Destroy(T** p) {
    Swap<T*>(*((T**)pMem)++, *p);
}

template <typename T>
inline void PtrListDL<T>::Destroy(T* p) {
    T** memPtr = (T**)pMem;
    while (*memPtr != NULL) {
        if (*memPtr == p) {
            Destroy(memPtr);
            break;
        }
        memPtr++;
    }
}

#endif COMMON_PTRLISTDL