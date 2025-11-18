/**
 * @file test_parser.c
 * @brief Unit tests for the Determa Parser
 */

#include "test_parser.h"
#include "test.h"    
#include "parser.h"  
#include "ast.h"
#include <stdlib.h>  
#include <stdio.h>

// Helper to extract the first statement's expression
static AstNode* get_first_stmt_expr(AstNode* root) {
    if (root == NULL) {
        printf("  DEBUG: Root is NULL\n");
        return NULL;
    }
    if (root->type != NODE_PROGRAM) {
        printf("  DEBUG: Root is not PROGRAM (Type: %d)\n", root->type);
        return NULL;
    }

    AstNodeProgram* p = (AstNodeProgram*)root;
    // printf("  DEBUG: Program has %d statements\n", p->statement_count);
    
    if (p->statement_count == 0) {
        printf("  DEBUG: Program has 0 statements\n");
        return NULL;
    }
    
    AstNode* stmt = p->statements[0];
    // printf("  DEBUG: First statement type is %d\n", stmt->type);

    // Unwrap EXPR_STMT if present
    if (stmt->type == NODE_EXPR_STMT) {
        AstNodeExprStmt* exprStmt = (AstNodeExprStmt*)stmt;
        // printf("  DEBUG: Unwrapping EXPR_STMT to get expression (Type: %d)\n", exprStmt->expression->type);
        return exprStmt->expression;
    } 
    else {
        // If it's a PrintStmt, VarDecl, etc., return it directly
        return stmt;
    }
}

void test_parser_integer_literal() {
    const char* source = "123;"; 
    AstNode* root = parse(source, 0); 
    
    // Print the tree first to verify structure
    printf("--- AST for '123;' ---\n");
    print_ast(root);

    AstNode* ast = get_first_stmt_expr(root);
    CHECK(ast != NULL, "Found statement");
    if(ast == NULL) { free_ast(root); return; }

    // Debug: Print what we actually got back
    // printf("  DEBUG: 'ast' type is %d (Expected %d)\n", ast->type, NODE_INT_LITERAL);

    CHECK(ast->type == NODE_INT_LITERAL, "AST node is an INT_LITERAL");
    if (ast->type == NODE_INT_LITERAL) {
        AstNodeIntLiteral* int_node = (AstNodeIntLiteral*)ast;
        CHECK(int_node->value == 123, "Integer value is 123");
    }
    free_ast(root);
}

/**
 * @brief Tests parsing a simple binary operation (1 + 2)
 */
void test_parser_simple_binary_op() {
    const char* source = "1 + 2;";
    AstNode* root = parse(source, 0);
    
    printf("--- AST for '1 + 2;' ---\n");
    print_ast(root);

    AstNode* ast = get_first_stmt_expr(root);
    
    CHECK(ast != NULL, "Found statement");
    if(ast == NULL) { free_ast(root); return; }

    CHECK(ast->type == NODE_BINARY_OP, "Root node is BINARY_OP");
    if (ast->type == NODE_BINARY_OP) {
        AstNodeBinaryOp* op = (AstNodeBinaryOp*)ast;
        CHECK(op->op.type == TOKEN_PLUS, "Operator is TOKEN_PLUS");

        AstNode* left_node = op->left;
        CHECK(left_node->type == NODE_INT_LITERAL, "Left child is INT_LITERAL");
    }
    free_ast(root);
}

/**
 * @brief Tests operator precedence (1 + 2 * 3)
 */
void test_parser_operator_precedence() {
    const char* source = "1 + 2 * 3;";
    AstNode* root = parse(source, 0);

    printf("--- AST for '1 + 2 * 3;' ---\n");
    print_ast(root);

    AstNode* ast = get_first_stmt_expr(root);

    CHECK(ast != NULL, "Found statement");
    if(ast == NULL) { free_ast(root); return; }

    CHECK(ast->type == NODE_BINARY_OP, "Root node is BINARY_OP");
    if (ast->type == NODE_BINARY_OP) {
        AstNodeBinaryOp* root_op = (AstNodeBinaryOp*)ast;
        CHECK(root_op->op.type == TOKEN_PLUS, "Root operator is TOKEN_PLUS");

        AstNodeBinaryOp* right_op = (AstNodeBinaryOp*)root_op->right;
        CHECK(right_op->node.type == NODE_BINARY_OP, "Right child is BINARY_OP");
    }
    free_ast(root);
}

void test_pda_debug_output() {
    const char* source = "(1 + 2);";
    printf("--- Printing PDA Debug Trace for '(1 + 2);' ---\n");
    AstNode* ast = parse(source, 1);
    CHECK(ast != NULL, "Parser returned non-NULL AST");
    free_ast(ast);
}

void test_parser_var_declaration() {
    const char* source = "var x = 10;";
    AstNode* root = parse(source, 0);

    AstNodeProgram* prog = (AstNodeProgram*)root;
    CHECK(prog->statement_count == 1, "Program has 1 statement");
    
    AstNode* stmt = prog->statements[0];
    CHECK(stmt->type == NODE_VAR_DECL, "Statement is VAR_DECL");
    
    AstNodeVarDecl* decl = (AstNodeVarDecl*)stmt;
    CHECK(decl->name.length == 1, "Var name length is 1");
    
    CHECK(decl->init->type == NODE_INT_LITERAL, "Initializer is INT_LITERAL");
    
    free_ast(root);
}

void test_parser_print_statement() {
    const char* source = "print 5 + 5;";
    AstNode* root = parse(source, 0);
    
    // Print statement is NOT wrapped in EXPR_STMT, so get_first_stmt_expr returns it directly
    AstNode* stmt = get_first_stmt_expr(root);
    
    CHECK(stmt->type == NODE_PRINT_STMT, "Statement is PRINT_STMT");
    free_ast(root);
}

void test_parser_program() {
    const char* source = 
        "var x = 1;\n"
        "var y = 2;\n"
        "print x + y;";
        
    AstNode* root = parse(source, 0);
    AstNodeProgram* prog = (AstNodeProgram*)root;
    
    CHECK(prog->statement_count == 3, "Program has 3 statements");
    CHECK(prog->statements[0]->type == NODE_VAR_DECL, "Stmt 1 is VAR_DECL");
    CHECK(prog->statements[2]->type == NODE_PRINT_STMT, "Stmt 3 is PRINT_STMT");
    
    free_ast(root);
}