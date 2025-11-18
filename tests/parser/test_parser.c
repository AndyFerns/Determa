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
    
    // Run with debug mode off (0)
    AstNode* ast = parse(source, 0); 
    
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

/**
 * @brief Tests parsing a simple binary operation (1 + 2)
 */
void test_parser_simple_binary_op() {
    const char* source = "1 + 2;";
    AstNode* ast = parse(source, 0);

    CHECK(ast != NULL, "Parser returned a non-NULL AST");
    if (ast == NULL) return;

    // Check root node
    CHECK(ast->type == NODE_BINARY_OP, "Root node is BINARY_OP");
    AstNodeBinaryOp* op = (AstNodeBinaryOp*)ast;
    CHECK(op->op.type == TOKEN_PLUS, "Operator is TOKEN_PLUS");

    // Check left child
    AstNodeIntLiteral* left = (AstNodeIntLiteral*)op->left;
    CHECK(left->node.type == NODE_INT_LITERAL, "Left child is INT_LITERAL");
    CHECK(left->value == 1, "Left child value is 1");

    // Check right child
    AstNodeIntLiteral* right = (AstNodeIntLiteral*)op->right;
    CHECK(right->node.type == NODE_INT_LITERAL, "Right child is INT_LITERAL");
    CHECK(right->value == 2, "Right child value is 2");

    printf("--- AST for '1 + 2;' ---\n");
    print_ast(ast);
    free_ast(ast);
}

/**
 * @brief Tests operator precedence (1 + 2 * 3)
 */
void test_parser_operator_precedence() {
    const char* source = "1 + 2 * 3;";
    AstNode* ast = parse(source, 0);

    CHECK(ast != NULL, "Parser returned a non-NULL AST");
    if (ast == NULL) return;

    // Root should be '+'
    CHECK(ast->type == NODE_BINARY_OP, "Root node is BINARY_OP");
    AstNodeBinaryOp* root = (AstNodeBinaryOp*)ast;
    CHECK(root->op.type == TOKEN_PLUS, "Root operator is TOKEN_PLUS");

    // Left child of '+' should be '1'
    AstNodeIntLiteral* left = (AstNodeIntLiteral*)root->left;
    CHECK(left->node.type == NODE_INT_LITERAL, "Left child is INT_LITERAL");
    CHECK(left->value == 1, "Left child value is 1");

    // Right child of '+' should be '(2 * 3)'
    AstNodeBinaryOp* right_op = (AstNodeBinaryOp*)root->right;
    CHECK(right_op->node.type == NODE_BINARY_OP, "Right child is BINARY_OP");
    CHECK(right_op->op.type == TOKEN_STAR, "Right child operator is TOKEN_STAR");

    // --- FIX: The typo was here, causing the crash ---
    // Check children of '*'
    AstNode* left_of_star_node = right_op->left;
    AstNode* right_of_star_node = right_op->right; // <-- Was initializing to itself!

    // Check left side of '*'
    CHECK(left_of_star_node->type == NODE_INT_LITERAL, "Left of '*' is INT_LITERAL");
    if(left_of_star_node->type == NODE_INT_LITERAL) {
         CHECK(((AstNodeIntLiteral*)left_of_star_node)->value == 2, "Left of '*' is 2");
    }

    // Check right side of '*'
    CHECK(right_of_star_node->type == NODE_INT_LITERAL, "Right of '*' is INT_LITERAL");
    if(right_of_star_node->type == NODE_INT_LITERAL) {
         CHECK(((AstNodeIntLiteral*)right_of_star_node)->value == 3, "Right of '*' is 3");
    }
    // --- END OF FIX ---


    printf("--- AST for '1 + 2 * 3;' (Precedence) ---\n");
    print_ast(ast);
    free_ast(ast);
}

/**
 * @brief Tests the PDA debug output
 */
void test_pda_debug_output() {
    const char* source = "(1 + 2);";
    printf("--- Printing PDA Debug Trace for '(1 + 2);' ---\n");
    // Run with debug mode on (1)
    AstNode* ast = parse(source, 1);
    
    CHECK(ast != NULL, "Parser returned non-NULL AST");
    
    free_ast(ast);
}