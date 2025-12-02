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



void test_parser_bool_literals() {
    // Test 'true'
    const char* source1 = "true;";
    AstNode* root1 = parse(source1, 0);
    AstNode* ast1 = get_first_stmt_expr(root1);
    
    CHECK(ast1->type == NODE_BOOL_LITERAL, "AST is BOOL_LITERAL");
    if (ast1->type == NODE_BOOL_LITERAL) {
        CHECK(((AstNodeBoolLiteral*)ast1)->value == 1, "true has value 1");
    }
    free_ast(root1);

    // Test 'false'
    const char* source2 = "false;";
    AstNode* root2 = parse(source2, 0);
    AstNode* ast2 = get_first_stmt_expr(root2);
    
    CHECK(ast2->type == NODE_BOOL_LITERAL, "AST is BOOL_LITERAL");
    if (ast2->type == NODE_BOOL_LITERAL) {
        CHECK(((AstNodeBoolLiteral*)ast2)->value == 0, "false has value 0");
    }
    free_ast(root2);
}

void test_parser_logic_precedence() {
    // This is the big one: "1 + 2 < 3 == true"
    // Precedence Order: (+) -> (<) -> (==)
    // Structure: ( (1 + 2) < 3 ) == true
    
    const char* source = "1 + 2 < 3 == true;";
    AstNode* root = parse(source, 0);
    AstNode* ast = get_first_stmt_expr(root);

    // 1. Root should be '=='
    CHECK(ast->type == NODE_BINARY_OP, "Root is BINARY_OP");
    AstNodeBinaryOp* eq = (AstNodeBinaryOp*)ast;
    CHECK(eq->op.type == TOKEN_EQUAL_EQUAL, "Root op is ==");

    // 2. Right side should be 'true'
    CHECK(eq->right->type == NODE_BOOL_LITERAL, "Right of == is BOOL");

    // 3. Left side should be '<'
    CHECK(eq->left->type == NODE_BINARY_OP, "Left of == is BINARY_OP");
    AstNodeBinaryOp* lt = (AstNodeBinaryOp*)eq->left;
    CHECK(lt->op.type == TOKEN_LESS, "Left op is <");

    // 4. Left side of '<' should be '+'
    CHECK(lt->left->type == NODE_BINARY_OP, "Left of < is BINARY_OP");
    AstNodeBinaryOp* add = (AstNodeBinaryOp*)lt->left;
    CHECK(add->op.type == TOKEN_PLUS, "Left of < op is +");

    // 5. Right side of '<' should be '3'
    CHECK(lt->right->type == NODE_INT_LITERAL, "Right of < is INT");

    printf("--- AST for Logic Precedence ---\n");
    print_ast(root);

    free_ast(root);
}


void test_parser_block() {
    const char* source = "{ var x = 1; }";
    AstNode* root = parse(source, 0);
    
    AstNodeProgram* prog = (AstNodeProgram*)root;
    CHECK(prog->statement_count == 1, "Program has 1 statement");
    
    AstNode* stmt = prog->statements[0];
    CHECK(stmt->type == NODE_BLOCK, "Statement is BLOCK");
    
    AstNodeBlock* block = (AstNodeBlock*)stmt;
    CHECK(block->statement_count == 1, "Block has 1 inner statement");
    CHECK(block->statements[0]->type == NODE_VAR_DECL, "Inner statement is VAR_DECL");
    
    free_ast(root);
}

void test_parser_if() {
    // Note: No parens around condition!
    const char* source = "if true { print 1; } else { print 2; }";
    AstNode* root = parse(source, 0);
    
    AstNodeProgram* prog = (AstNodeProgram*)root;
    AstNode* stmt = prog->statements[0];
    CHECK(stmt->type == NODE_IF, "Statement is IF");
    
    AstNodeIf* ifNode = (AstNodeIf*)stmt;
    CHECK(ifNode->condition->type == NODE_BOOL_LITERAL, "Condition is BOOL");
    CHECK(ifNode->thenBranch->type == NODE_BLOCK, "Then branch is BLOCK");
    CHECK(ifNode->elseBranch->type == NODE_BLOCK, "Else branch is BLOCK");
    
    free_ast(root);
}

void test_parser_while() {
    const char* source = "while x < 10 { print x; }";
    AstNode* root = parse(source, 0);
    
    AstNodeProgram* prog = (AstNodeProgram*)root;
    AstNode* stmt = prog->statements[0];
    CHECK(stmt->type == NODE_WHILE, "Statement is WHILE");
    
    AstNodeWhile* whileNode = (AstNodeWhile*)stmt;
    CHECK(whileNode->condition->type == NODE_BINARY_OP, "Condition is BINARY_OP (<)");
    CHECK(whileNode->body->type == NODE_BLOCK, "Body is BLOCK");
    
    free_ast(root);
}