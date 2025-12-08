/**
 * @file test_vm.c
 * @brief Unit tests for the Virtual Machine backend (Function-based VM).
 */

#include <string.h>

#include "test_vm.h"
#include "test.h"

#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/value.h"
#include "vm/object.h"
#include "vm/compiler.h"
#include "parser.h"
#include "typechecker.h"
#include "ast.h"

/* -------------------------------------------------------------
 * Helper: compile + run a snippet that RETURNS a value
 * ------------------------------------------------------------- */
static Value run_and_get_return(const char* source) {
    AstNode* ast = parse(source, 0);
    CHECK(ast != NULL, "Parse must succeed");

    CHECK(typecheck_ast(ast), "Typecheck must succeed");

    ObjFunction* fn = compile_ast(ast);
    CHECK(fn != NULL, "Compilation must succeed");

    init_vm();
    interpret(fn);

    // Now that runtime has finished, the return value is on stack[0]
    CHECK(vm.stackTop > vm.stack, "Return value must exist on stack");
    Value v = vm.stack[0];

    free_vm();
    free_ast(ast);
    return v;
}


/* -------------------------------------------------------------
 * TEST 1: Basic chunk operations (unchanged)
 * ------------------------------------------------------------- */
static void test_vm_initialization() {
    Chunk chunk;
    init_chunk(&chunk);

    CHECK(chunk.count == 0, "Chunk empty");
    CHECK(chunk.capacity == 0, "Capacity=0");
    CHECK(chunk.code == NULL, "Code=NULL");

    int idx = add_constant(&chunk, INT_VAL(123));
    CHECK(idx == 0, "First constant index = 0");
    CHECK(AS_INT(chunk.constants.values[0]) == 123, "Constant OK");

    write_chunk(&chunk, OP_RETURN, 1);
    CHECK(chunk.count == 1, "1 byte written");
    CHECK(chunk.code[0] == OP_RETURN, "Opcode is OP_RETURN");

    free_chunk(&chunk);
}


/* -------------------------------------------------------------
 * TEST 2: Arithmetic using return-value wrapper
 * ------------------------------------------------------------- */
static void test_vm_arithmetic() {
    Value v = run_and_get_return("return 1 + 2;");

    CHECK(IS_INT(v), "Arithmetic result should be integer");
    CHECK(AS_INT(v) == 3, "1 + 2 must equal 3");
}


/* -------------------------------------------------------------
 * TEST 3: Operator precedence using return-value wrapper
 * ------------------------------------------------------------- */
static void test_vm_precedence_manual() {
    Value v = run_and_get_return("return -5 + 10;");

    CHECK(IS_INT(v), "Precedence result should be integer");
    CHECK(AS_INT(v) == 5, "-5 + 10 must equal 5");
}


/* -------------------------------------------------------------
 * TEST 4: Manual string allocation (unchanged)
 * ------------------------------------------------------------- */
static void test_vm_manual_string_alloc() {
    init_vm();

    ObjString* s1 = copy_string("Hello", 5);
    CHECK(s1 != NULL, "String alloc OK");
    CHECK(strcmp(s1->chars, "Hello") == 0, "String content OK");

    ObjString* s2 = copy_string("World", 5);
    CHECK(vm.objects == (Obj*)s2, "Newest object is list head");

    free_vm();
    CHECK(vm.objects == NULL, "Object list cleared");
}


/* -------------------------------------------------------------
 * SUITE WRAPPER
 * ------------------------------------------------------------- */
void test_vm_suite() {
    run_test(test_vm_initialization,        "VM - Initialization");
    run_test(test_vm_arithmetic,            "VM - Arithmetic via RETURN");
    run_test(test_vm_precedence_manual,     "VM - Operator precedence via RETURN");
    run_test(test_vm_manual_string_alloc,   "VM - Manual string alloc");
}
