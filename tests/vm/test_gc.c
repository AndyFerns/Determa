/**
 * @file test_gc.c
 * @brief Unit tests for the Garbage Collector.
 */

#include "test_gc.h"
#include "test.h"
#include "vm/vm.h"
#include "vm/memory.h"
#include "vm/object.h"
#include "vm/value.h"

#include <string.h>
#include <stdio.h>

// Static tests for internal use

static void test_gc_basics() {
    init_vm(); // Ensure clean state
    
    // 1. Create "Garbage" (Unreferenced Object)
    // We create a string, but we DO NOT push it to the stack or globals.
    // It exists only in the C variable 'str' and the VM's linked list.
    ObjString* str = copy_string("garbage", 7);
    
    size_t mem_before = vm.bytesAllocated;
    CHECK(mem_before > 0, "Memory allocated for string");
    CHECK(vm.objects == (Obj*)str, "String added to VM list");
    
    // 2. Run GC
    // Since 'str' is not on the stack or in globals, it is unreachable.
    collect_garbage();
    
    // 3. Verify Cleanup
    CHECK(vm.bytesAllocated < mem_before, "GC should free unreachable string");
    CHECK(vm.objects == NULL, "Heap list should be empty");
    
    free_vm();
}

static void test_gc_preservation() {
    init_vm();
    
    // 1. Create "Treasure" (Referenced Object)
    ObjString* str = copy_string("treasure", 8);
    Value val = OBJ_VAL(str);
    
    // 2. Protect it by pushing it to the Stack (Root)
    push(val);
    
    size_t mem_before = vm.bytesAllocated;
    
    // 3. Run GC
    collect_garbage();
    
    // 4. Verify Survival
    // It should still exist because the Stack points to it.
    CHECK(vm.bytesAllocated == mem_before, "GC should NOT free reachable string");
    CHECK(vm.objects == (Obj*)str, "String should remain in heap list");
    
    // 5. Unprotect (Pop) and Collect
    pop(); // Now it's garbage
    collect_garbage();
    
    // 6. Verify Deletion
    CHECK(vm.bytesAllocated < mem_before, "GC should free string after pop");
    CHECK(vm.objects == NULL, "Heap list should be empty");
    
    free_vm();
}

// Global suite entry point
void test_gc_suite() {
    run_test(test_gc_basics, "GC - Basic Collection (Sweep Garbage)");
    run_test(test_gc_preservation, "GC - Root Preservation (Mark & Sweep)");
}