/**
 * @file test_vm.c
 * @brief Unit tests for the Virtual Machine backend.
 */

#include "test_vm.h"
#include "test.h"
#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/opcode.h"

void test_vm_initialization() {
    // Test 1: Create and initialize a chunk
    Chunk chunk;
    init_chunk(&chunk);

    CHECK(chunk.count == 0, "Chunk count should be 0 initially");
    CHECK(chunk.capacity == 0, "Chunk capacity should be 0 initially");
    CHECK(chunk.code == NULL, "Chunk code should be NULL initially");

    // Test 2: Add a constant
    int constantIndex = add_constant(&chunk, 123);
    CHECK(constantIndex == 0, "First constant index should be 0");
    CHECK(chunk.constants.values[0] == 123, "Constant value should be 123");
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