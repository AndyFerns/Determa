/**
 * @file test_vm.c
 * @brief Unit tests for the Virtual Machine backend.
 */

#include <string.h>

#include "test_vm.h"
#include "test.h"

#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/value.h"   // needed for macroes
#include "vm/object.h" // --- NEW: Access to internal Object API

// Test functions are now static to avoid linker errors when building the test runner.

static void test_vm_initialization() {
    // Test 1: Create and initialize a chunk
    Chunk chunk;
    init_chunk(&chunk);

    CHECK(chunk.count == 0, "Chunk count should be 0 initially");
    CHECK(chunk.capacity == 0, "Chunk capacity should be 0 initially");
    CHECK(chunk.code == NULL, "Chunk code should be NULL initially");

    // Test 2: Add a constant
    int constantIndex = add_constant(&chunk, INT_VAL(123));
    CHECK(constantIndex == 0, "First constant index should be 0");
    CHECK(AS_INT(chunk.constants.values[0]) == 123, "Constant value should be 123");
    CHECK(chunk.constants.count == 1, "Constant pool count should be 1");

    // Test 3: Write an instruction
    write_chunk(&chunk, OP_RETURN, 1);
    CHECK(chunk.count == 1, "Chunk count should be 1");
    CHECK(chunk.code[0] == OP_RETURN, "First byte should be OP_RETURN");
    CHECK(chunk.lines[0] == 1, "Line number should be 1");

    // Test 4: Clean up
    free_chunk(&chunk);
    CHECK(chunk.count == 0, "Chunk count should be 0 after free");
    CHECK(chunk.code == NULL, "Chunk code should be NULL after free");
}


static void test_vm_arithmetic() {
    // Test: 1 + 2
    Chunk chunk;
    init_chunk(&chunk);
    init_vm();

    // 1. Push Constant 1
    int c1 = add_constant(&chunk, INT_VAL(1));
    write_chunk(&chunk, OP_CONSTANT, 1);
    write_chunk(&chunk, (uint8_t)c1, 1); // Cast constant index to uint8_t

    // 2. Push Constant 2
    int c2 = add_constant(&chunk, INT_VAL(2));
    write_chunk(&chunk, OP_CONSTANT, 1);
    write_chunk(&chunk, (uint8_t)c2, 1); // Cast constant index to uint8_t

    // 3. Add
    write_chunk(&chunk, OP_ADD, 1);

    // 4. Return
    write_chunk(&chunk, OP_RETURN, 1);

    // Run!
    interpret(&chunk);

    // Check Result
    // Stack should have 1 item: result 3
    Value result = peek(0);
    CHECK(IS_INT(result), "Result is INT");
    CHECK(AS_INT(result) == 3, "1 + 2 = 3");

    free_chunk(&chunk);
    free_vm();
}


static void test_vm_precedence_manual() {
    // Test: -5 + 10 (Order of ops check)
    Chunk chunk;
    init_chunk(&chunk);
    init_vm();

    // Push 5
    int c1 = add_constant(&chunk, INT_VAL(5));
    write_chunk(&chunk, OP_CONSTANT, 1);
    write_chunk(&chunk, (uint8_t)c1, 1);

    // Negate (-5)
    write_chunk(&chunk, OP_NEGATE, 1);

    // Push 10
    int c2 = add_constant(&chunk, INT_VAL(10));
    write_chunk(&chunk, OP_CONSTANT, 1);
    write_chunk(&chunk, (uint8_t)c2, 1);

    // Add
    write_chunk(&chunk, OP_ADD, 1);
    write_chunk(&chunk, OP_RETURN, 1);

    interpret(&chunk);

    Value result = peek(0);
    CHECK(AS_INT(result) == 5, "-5 + 10 should equal 5");

    free_chunk(&chunk);
    free_vm();
}

static void test_vm_manual_string_alloc() {
    init_vm(); // Ensure VM state is clean (vm.objects = NULL)

    // 1. Manually allocate a string using internal API
    ObjString* str = copy_string("Hello", 5);

    // 2. Verify Allocation and Type
    CHECK(str != NULL, "String allocation returned non-NULL");
    CHECK(str->length == 5, "String length is correct");
    CHECK(strcmp(str->chars, "Hello") == 0, "String content is correct");
    CHECK(str->obj.type == OBJ_STRING, "Object type is OBJ_STRING");

    // 3. Verify VM Tracking (The "Tracker List")
    // The new object should be at the head of the VM's object list
    CHECK(vm.objects == (Obj*)str, "VM is tracking the new object");

    // 4. Verify Value Wrapping
    Value val = OBJ_VAL(str);
    CHECK(IS_OBJ(val), "Value is tagged as Object");
    CHECK(IS_STRING(val), "Value is tagged as String");
    CHECK(AS_OBJ(val) == (Obj*)str, "Unwrapped object pointer matches");
    CHECK(strcmp(AS_CSTRING(val), "Hello") == 0, "Unwrapped C-String matches");

    // 5. Test Multiple Allocations (Linked List check)
    ObjString* str2 = copy_string("World", 5);
    CHECK(vm.objects == (Obj*)str2, "VM list head updated to newest object");
    CHECK(vm.objects->next == (Obj*)str, "New object points to old object");

    // 6. Clean up
    free_vm(); // Should free both strings without crashing
    CHECK(vm.objects == NULL, "VM object list cleared after free_vm");
}


// Global function to run tests (called by test_runner.c)
void test_vm_suite() {
    run_test(test_vm_initialization, "VM - Chunk & Constant Pool Basics");
    run_test(test_vm_arithmetic, "VM - Arithmetic (1 + 2)");
    run_test(test_vm_precedence_manual, "VM - Manual Precedence (-5 + 10)");
    run_test(test_vm_manual_string_alloc, "VM - Manual String Allocation & Tracking");
}