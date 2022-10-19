#include "types.h"
#include "common/Heap.h"

extern int gEmptyPtrList[2];

template <typename T>
struct PtrList {
    T **pPointers;
    void Init(int);
	void Destroy(T*);
};

template <typename T>
inline void PtrList<T>::Init(int count) {
	if (count == 0) {
		pPointers = (T**)&gEmptyPtrList[1];
	} else {
		pPointers = (T**)Heap_MemAlloc((count + 2) * 4);
        *pPointers = NULL;
        int i = 0;
        for (i; i < count; i++) {
            *(++pPointers) = (T*)0xdeadbeef;
        }
        pPointers++;
        *pPointers = NULL;
    }
}