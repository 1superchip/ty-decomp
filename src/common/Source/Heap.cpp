#include "common/Heap.h"
#include "types.h"
#include "Dolphin/os/OSAlloc.h"

extern "C" int OSGetConsoleSimulatedMemSize(void);
extern "C" void printf(char*, ...);
extern "C" void exit(int);


static bool heapInitialised = false;

void Heap_InitModule(int arg0) {
	heapInitialised = true;
}

void Heap_DeinitModule(void) {
	heapInitialised = false;
}

/// @brief Returns the current amount of used memory
/// @param  None
/// @return Current amount of used memory
int Heap_MemoryUsed(void) {
    return OSGetConsoleSimulatedMemSize() - OSCheckHeap(0);
}

/// @brief Allocates a block of memory, does not use alignment
/// @param size size of block to allocate
/// @param alignment Unused parameter for alignment
/// @return Allocated block of memory
void* Heap_MemAllocAligned(int size, int alignment) {
    return OSAllocFromHeap(__OSCurrHeap, size);
}

/// @brief Frees an aligned memory block
/// @param ptr Memory to free
void Heap_MemFreeAligned(void* ptr) {
    OSFreeToHeap(__OSCurrHeap, ptr);
}

void* Heap_MemAlloc(int size) {
    int simulatedMemSize;
    void* pMem;

    pMem = OSAllocFromHeap(__OSCurrHeap, size);
    if (pMem == NULL) {
        printf("Heap Alloc Failed: %d bytes attempted, %d used\n", size, OSGetConsoleSimulatedMemSize() - OSCheckHeap(0));
    }
    return pMem;
}

void Heap_MemFree(void* ptr) {
    OSFreeToHeap(__OSCurrHeap, ptr);
}

/// @brief Checks the heap
/// @param file Filename to print
/// @param lineNumber Line number to print
/// @return Returns 0 if no error occurs otherwise exits the program
int Heap_Check(char* file, int lineNumber) {
    if ((int)OSCheckHeap(0) == -1) {
        printf("\nCorrupt HEAP\n");
        printf("%s Line: %d\n", file, lineNumber);
        exit(0);
    }
    return 0;
}
