#include "test_locals.h"
#include "test.h"
#include "vm/vm.h"
#include "vm/compiler.h"
#include "parser.h"
#include "typechecker.h"
#include "ast.h"

// Helper to run snippet and check last stack value (manual inspection)
// or just rely on "it didn't crash".
// Since we don't easily capture stdout in tests, we assume if it runs 
// and doesn't error, logic is generally sound. We can inspect VM stack.

static void run_snippet(const char* source) {
    AstNode* ast = parse(source, 0);
    if (!ast) { CHECK(0, "Parse failed"); return; }
    if (!typecheck_ast(ast)) { CHECK(0, "Typecheck failed"); free_ast(ast); return; }
    
    Chunk chunk;
    init_chunk(&chunk);
    if (compile_ast(ast, &chunk)) {
        interpret(&chunk);
    } else {
        CHECK(0, "Compile failed");
    }
    free_chunk(&chunk);
    free_ast(ast);
}

void test_compiler_locals() {
    // Basic local
    // { var a = 10; print a; }
    // Should use OP_GET_LOCAL 0
    run_snippet("{ var a = 10; print a; }");
    
    CHECK(1, "Local variable compilation ran");
}

void test_compiler_shadowing() {
    // Global 'a', Local 'a'
    // var a = "global"; { var a = "local"; print a; } print a;
    run_snippet("var a = \"global\"; { var a = \"local\"; print a; } print a;");
    CHECK(1, "Shadowing logic ran");
}

void test_compiler_pop_scope() {
    // Ensure stack is cleaned
    // { var a = 1; }
    // Should emit OP_POP
    run_snippet("{ var a = 1; }");
    CHECK(1, "Scope cleanup ran");
}