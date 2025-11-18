/**
 * @file test_typechecker.c
 * @author Andrew Fernandes
 * @brief Testcases for typechecking
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "test_typechecker.h"
#include "test.h"
#include "parser.h"
#include "typechecker.h"
#include "ast.h"

void test_tc_var_decl_and_access() {
    // Valid code
    const char* source = "var x = 10; print x + 1;";
    AstNode* root = parse(source, 0);
    
    CHECK(root != NULL, "Parse failed");
    if (root) {
        int valid = typecheck_ast(root);
        CHECK(valid == 1, "Valid code should pass type checking");
        free_ast(root);
    }
}

void test_tc_undefined_var() {
    // Invalid code: 'y' is not defined
    const char* source = "var x = 10; print y;";
    AstNode* root = parse(source, 0);
    
    CHECK(root != NULL, "Parse failed");
    if (root) {
        printf("--- Expecting Type Error Below ---\n");
        int valid = typecheck_ast(root);
        printf("----------------------------------\n");
        CHECK(valid == 0, "Undefined variable should fail type checking");
        free_ast(root);
    }
}

void test_tc_redeclaration() {
    // Invalid code: 'x' defined twice
    const char* source = "var x = 10; var x = 20;";
    AstNode* root = parse(source, 0);
    
    CHECK(root != NULL, "Parse failed");
    if (root) {
        printf("--- Expecting Type Error Below ---\n");
        int valid = typecheck_ast(root);
        printf("----------------------------------\n");
        CHECK(valid == 0, "Redeclaration should fail type checking");
        free_ast(root);
    }
}

void test_tc_binary_op_mismatch() {
    // For now we only have INT, so it's hard to mismatch types unless we use VOID
    // But logic is ready
}