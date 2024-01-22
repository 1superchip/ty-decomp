#ifndef COMMON_PTRLIST
#define COMMON_PTRLIST

#include "types.h"
#include "common/Heap.h"

extern int gEmptyPtrList[2];

template <typename T>
struct PtrList {
    T **pPointers;
    void Init(int);
	void Destroy(T*);
    void Deinit(void);
    T* AddEntry(T* p) {
        return *--pPointers = p;
    }
    T* GetUnkEntry(void) {
        return *pPointers++;
    }
	T** GetPointers(void) {
		return pPointers;
	}
};

template <typename T>
inline void PtrList<T>::Init(int count) {
	if (count == 0) {
		pPointers = (T**)&gEmptyPtrList[1];
	} else {
		pPointers = (T**)Heap_MemAlloc((count + 2) * sizeof(T*));
        *pPointers = NULL;
        int i = 0;
        for (i; i < count; i++) {
            *++pPointers = (T*)0xdeadbeef;
        }
        *++pPointers = NULL;
    }
}

template <typename T>
inline void PtrList<T>::Deinit(void) {
    while (*--pPointers != NULL) {}
    if (pPointers != (T**)&gEmptyPtrList) {
        Heap_MemFree(pPointers);
    }
    pPointers = NULL;
}

template <typename T>
void PtrList<T>::Destroy(T* pTemplate) {
    T** ptrs = pPointers;
    while (*ptrs != NULL) {
        if (*ptrs == pTemplate) {
            *ptrs = GetUnkEntry();
            return;
        }
        ptrs++;
    }
}

#endif // COMMON_PTRLIST
