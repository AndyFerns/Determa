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

void mark_object(Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return; // Already visited

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    // Add to gray stack (worklist)
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = (vm.grayCapacity < 8) ? 8 : vm.grayCapacity * 2;
        // Note: We use system realloc here to avoid infinite GC loops
        vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
        if (vm.grayStack == NULL) exit(1);
    }

    vm.grayStack[vm.grayCount++] = object;
}

void mark_value(Value value) {
    if (IS_OBJ(value)) mark_object(AS_OBJ(value));
}


static void mark_roots() {
    // 1. Mark the Stack
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        mark_value(*slot);
    }

    // 2. Mark Globals
    for (int i = 0; i < GLOBALS_MAX; i++) {
        mark_value(vm.globals[i]);
    }
    
    // 3. Mark the Compiler's roots (if we were compiling)
    // For now, we assume GC only runs during runtime interpret().
}

static void blacken_object(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_STRING:
            // Strings have no outgoing references, so nothing to do.
            break;
    }
}

static void trace_references() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blacken_object(object);
    }
}


static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;

    while (object != NULL) {
        if (object->isMarked) {
            // It survived! Unmark for next cycle
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            // It's garbage. Unlink and free.
            Obj* unreached = object;
            object = object->next;
            
            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object; // We freed the head
            }

#ifdef DEBUG_LOG_GC
            printf("%p free ", (void*)unreached);
            print_value(OBJ_VAL(unreached));
            printf("\n");
#endif

            free_object(unreached);
        }
    }
}

void collect_garbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin --\n");
    size_t before = vm.bytesAllocated;
#endif

    mark_roots();
    trace_references();
    sweep();

    // Adjust threshold for next run
    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end --\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}


void free_object(Obj* object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            // only using FREE_ARRAY macro in chunk.c, but do manual
            // account keeping or just raw free.
            // we should ideally use reallocate(ptr, size, 0).
            // But we don't track individual string size easily here without casting.
            
            size_t size = sizeof(ObjString) + string->length + 1;
            free(string->chars);
            free(string);
            vm.bytesAllocated -= size;
            break;
        }
    }
}