/**
 * @file memory.c
 * @author Andrew Fernandes
 * @brief Implementation of Mark-and-Sweep Garbage Collection.
 * @version 0.1
 * @date 2025-11-30
 *
 * This file implements a non-moving, mark-and-sweep garbage collector
 * for the Determa VM. Allocation is handled through the @ref reallocate()
 * wrapper, which tracks memory usage and triggers collection when needed.
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
// #define DEBUG_STRESS_GC

/**
 * @brief Reallocation wrapper that tracks memory usage and triggers GC when needed.
 *
 * This replaces all raw calls to malloc(), realloc(), and free()
 * inside the VM. If `newSize` is greater than `oldSize`, the VM's
 * allocation counter increases. If GC stress testing is enabled,
 * a collection is triggered before allocating new memory.
 *
 * @param pointer The old memory pointer (may be NULL).
 * @param oldSize Size previously allocated for this pointer.
 * @param newSize Requested new size. If 0, memory is freed.
 * @return void* Pointer to newly allocated region, or NULL if freed.
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;

    if (newSize > oldSize) {
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

/**
 * @brief Mark a heap object and enqueue it into the gray stack.
 *
 * If the object is already marked, this does nothing. Otherwise it becomes
 * marked and appended to the gray stack for further traversal in the mark
 * phase.
 *
 * @param object Pointer to the object to mark.
 */
void mark_object(Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    // Add to gray stack
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = (vm.grayCapacity < 8) ? 8 : vm.grayCapacity * 2;
        vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
        if (vm.grayStack == NULL) exit(1);
    }

    vm.grayStack[vm.grayCount++] = object;
}

/**
 * @brief Mark a Value if it is an object.
 *
 * @param value The value to mark.
 */
void mark_value(Value value) {
    if (IS_OBJ(value)) mark_object(AS_OBJ(value));
}

/**
 * @brief Mark all GC root references.
 *
 * This includes:
 * - Stack values
 * - Globals
 * - (Optionally) compiler roots
 *
 * These objects will become starting points for reachability.
 */
static void mark_roots() {
    // 1. Stack values
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        mark_value(*slot);
    }

    // 2. Global variables
    for (int i = 0; i < GLOBALS_MAX; i++) {
        mark_value(vm.globals[i]);
    }

    // 3. Compiler roots (when GC runs during compile)
    mark_compiler_roots();
}

/**
 * @brief Process a single object from the gray stack.
 *
 * This function is responsible for recursively marking referenced objects.
 * For now only strings exist, which have no inner references.
 *
 * @param object The object to process.
 */
static void blacken_object(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_STRING:
            // Strings have no outgoing references.
            break;
    }
}

/**
 * @brief Trace all reachable objects by processing the gray stack.
 *
 * Repeatedly removes objects from the gray stack and blackens them
 * until no gray objects remain.
 */
static void trace_references() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blacken_object(object);
    }
}

/**
 * @brief Sweep through the heap and free all unmarked objects.
 *
 * Objects that remain unmarked after tracing are unreachable
 * and are therefore freed. Surviving objects are unmarked
 * for the next GC cycle.
 */
static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;

    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;

            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object;
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

/**
 * @brief Perform a complete mark-and-sweep garbage collection cycle.
 *
 * Steps:
 * 1. Mark roots.
 * 2. Trace reachable references.
 * 3. Sweep unreachable objects.
 * 4. Recalculate next GC threshold.
 */
void collect_garbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin --\n");
    size_t before = vm.bytesAllocated;
#endif

    mark_roots();
    trace_references();
    sweep();

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end --\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

/**
 * @brief Free a heap object according to its type.
 *
 * This function is called only during sweep() when an object is
 * confirmed to be unreachable.
 *
 * @param object The heap object to free.
 */
void free_object(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif

    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            
            // Calculate size to subtract from tracker
            // size_t size = sizeof(ObjString) + string->length + 1;
            
            // NEW: Use reallocate to free so accounting matches allocation
            reallocate(string->chars, string->length + 1, 0);
            reallocate(string, sizeof(ObjString), 0);
            break;
        }
    }
}