/**
 * @file memory.c
 * @author Andrew Fernandes
 * @brief Implementation of Mark-and-Sweep Garbage Collection.
 * @version 0.1
 * @date 2025-11-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include "vm/memory.h"
#include "vm/vm.h"
#include "vm/compiler.h" // Needed if we want to mark compiler roots later

#define GC_HEAP_GROW_FACTOR 2

// Toggle this to see GC logs in the terminal
// #define DEBUG_LOG_GC

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;

    if (newSize > oldSize) {
        // If we are stressing memory, trigger GC *before* allocating more
        #ifdef DEBUG_STRESS_GC
        collect_garbage();
        #endif

        if (vm.bytesAllocated > vm.nextGC) {
            collect_garbage();
        }
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) {
        fprintf(stderr, "Fatal: Out of memory.\n");
        exit(1);
    }
    return result;
}
