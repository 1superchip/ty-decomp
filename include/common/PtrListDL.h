#include "types.h"
#include "common/Heap.h"

extern int gEmptyPtrListDL[2]; // Utils.cpp

// should this be a template?
// inlines of IsFull() and IsEmpty()
// void PtrListDL<T>InitSized(char*,int,int);
// is InitSized what DDASession uses?
// Init calls InitSized
template <typename T>
struct PtrListDL {
    T *pMem;
    void Init(int, int);
	void Deinit(void);
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
};

template <typename T>
inline void PtrListDL<T>::Init(int count, int size) {
    // count * size = structure array size
    // count * 4 = pointer array size
    // 8 byte header for memory pointer and (name pointer?)
    // name pointer most likely unused
    pMem = (T*)Heap_MemAlloc(count * size + 8 + count * 4);
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
        while(*(HeatFlare**)ptrs != NULL) {
            if (*(T**)ptrs < (T*)memory) {
                memory = *(T**)ptrs;
            }
            ((T**)ptrs)++;
        }
        Heap_MemFree(memory);
    }
    pMem = NULL;
}