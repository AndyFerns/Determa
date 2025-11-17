/**
 * @file test_parser.c
 * @brief Unit tests for the Determa Parser
 */

#include "test_parser.h"
#include "test.h"     // For the CHECK macro
#include "parser.h"   // The module we are testing
#include "ast.h"
#include <stdlib.h>   // For NULL

/**
 * @brief Tests parsing a simple integer literal
 */
void test_parser_integer_literal() {
    const char* source = "123;"; // A simple program
    
    AstNode* ast = parse(source);
    
    // 1. Check that parsing succeeded
    CHECK(ast != NULL, "Parser returned a non-NULL AST");
    if (ast == NULL) return; // Guard for NULL dereference

    // 2. Check the node type
    CHECK(ast->type == NODE_INT_LITERAL, "AST node is an INT_LITERAL");

    // 3. Check the value
    if (ast->type == NODE_INT_LITERAL) {
        AstNodeIntLiteral* int_node = (AstNodeIntLiteral*)ast;
        CHECK(int_node->value == 123, "Integer value is 123");
    }

    // 4. Print the AST for manual inspection
    printf("--- AST for '123;' ---\n");
    print_ast(ast);
    
    // 5. Clean up
    free_ast(ast);
}