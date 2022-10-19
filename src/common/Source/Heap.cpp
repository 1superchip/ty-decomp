#include "common/Heap.h"
#include "types.h"
#include "Dolphin/os/OSAlloc.h"

extern "C" int OSGetConsoleSimulatedMemSize(void);
extern "C" void printf(char*, ...);
extern "C" void exit(int);


static bool heapInitialised;

void Heap_InitModule(int arg0) {
	heapInitialised = true;
}

void Heap_DeinitModule(void) {
	heapInitialised = false;
}

int Heap_MemoryUsed(void) {
    return OSGetConsoleSimulatedMemSize() - OSCheckHeap(0);
}

// alignment is unused!
void* Heap_MemAllocAligned(int size, int alignment) {
    return OSAllocFromHeap(__OSCurrHeap, size);
}

void Heap_MemFreeAligned(void* ptr) {
    OSFreeToHeap(__OSCurrHeap, ptr);
}

void* Heap_MemAlloc(int size) {
    int simulatedMemSize;
    void* pMem;

    pMem = OSAllocFromHeap(__OSCurrHeap, size);
    if (pMem == 0) {
        printf("Heap Alloc Failed: %d bytes attempted, %d used\n", size, OSGetConsoleSimulatedMemSize() - OSCheckHeap(0));
    }
    return pMem;
}

void Heap_MemFree(void* ptr) {
    OSFreeToHeap(__OSCurrHeap, ptr);
}

int Heap_Check(char* file, int lineNumber) {
    if ((int)OSCheckHeap(0) == -1) {
        printf("\nCorrupt HEAP\n");
        printf("%s Line: %d\n", file, lineNumber);
        exit(0);
    }
    return 0;
}