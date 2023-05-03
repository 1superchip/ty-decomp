#ifndef COMMON_HEAP
#define COMMON_HEAP

void Heap_InitModule(int);
void Heap_DeinitModule(void);
int Heap_MemoryUsed(void);
void* Heap_MemAllocAligned(int size, int alignment); // alignment is unused
void Heap_MemFreeAligned(void* ptr);
void* Heap_MemAlloc(int size);
void Heap_MemFree(void* ptr);
int Heap_Check(char* file, int lineNumber);

#endif // COMMON_HEAP