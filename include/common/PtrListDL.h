#include "common/Heap.h"

// should this be a template?
// inlines of IsFull() and IsEmpty()
// void PtrListDL<T>InitSized(char*,int,int);
// is InitSized what DDASession uses?
// Init calls InitSized
template <typename T>
struct PtrListDL {
    T *pMem;
    void Init(int, int);
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

// defining this in the struct with "-inline auto" does inline this function but defining it here doesn't auto inline
// need the inline def here
template <typename T>
inline void PtrListDL<T>::Init(int count, int size) {
    // count * size = structure array size
    // count * 4 = pointer array size
    // 8 byte header for memory pointer and (name pointer?)
    // name pointer most likely unused
    pMem = (T*)Heap_MemAlloc(count * size + 8 + count * 4);
    T* memory = pMem;
    int* arrayEnd = (int*)((int)memory + count * size); // go to array memory end
    pMem = (T*)arrayEnd; // set mem pointer to end of array
    *arrayEnd = 0;
    int i = 0;
    while(i < count) {
        int* pNext = (int*)pMem + 1;
        pMem = (T*)pNext;
        *pNext = (int)memory;
        memory = ((T*)memory + 1);
        i++;
    }
    void* last = (void*)((int*)pMem + 1);
    pMem = (T*)last;
    *(int*)last = 0;
}