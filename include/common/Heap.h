#ifndef COMMON_HEAP
#define COMMON_HEAP

void Heap_InitModule(int);
void Heap_DeinitModule(void);
int Heap_MemoryUsed(void);
void* Heap_MemAllocAligned(int, int);
void Heap_MemFreeAligned(void*);
void* Heap_MemAlloc(int);
void Heap_MemFree(void*);
int Heap_Check(char*, int);

#endif // COMMON_HEAP