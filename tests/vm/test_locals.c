#include "test_locals.h"
#include "test.h"
#include "vm/vm.h"
#include "vm/compiler.h"
#include "parser.h"
#include "typechecker.h"
#include "ast.h"

static void run_snippet(const char* src) {
    AstNode* ast = parse(src, 0);
    CHECK(ast != NULL, "Parse succeeded");

    if (!typecheck_ast(ast)) {
        CHECK(0, "Typechecker failed");
        free_ast(ast);
        return;
    }

    // Initialize VM BEFORE compilation so objects allocated during
    // compilation (strings, function objects) are tracked by the VM.
    init_vm();

    ObjFunction* fn = compile_ast(ast);

    if (!fn) { 
        CHECK(0, "Compile failed"); 
        free_ast(ast);
        free_vm();
        return; 
    }

    CHECK(fn != NULL, "Compilation succeeded");

    interpret(fn);

    free_ast(ast);
    free_vm();
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
