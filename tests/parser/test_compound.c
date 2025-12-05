/**
 * @file test_compound.c
 * @brief Implementation of feature tests.
 */

#include "test_compound.h"
#include "test.h"
#include "parser.h"
#include "ast.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

// --- Helper to unwrap statements ---
static AstNode* get_first_stmt_expr(AstNode* root) {
    if (root == NULL || root->type != NODE_PROGRAM) return NULL;
    AstNodeProgram* p = (AstNodeProgram*)root;
    if (p->statement_count == 0) return NULL;
    AstNode* stmt = p->statements[0];
    if (stmt->type == NODE_EXPR_STMT) {
        return ((AstNodeExprStmt*)stmt)->expression;
    }
    return stmt; // Return directly if it's a VarAssign or other statement
}

void test_parser_modulo() {
    const char* source = "10 % 3;";
    AstNode* root = parse(source, 0);
    AstNode* ast = get_first_stmt_expr(root);

    CHECK(ast != NULL, "AST exists");
    
    // 1. Verify it is a Binary Op
    CHECK(ast->type == NODE_BINARY_OP, "Type is BINARY_OP");
    
    if (ast->type == NODE_BINARY_OP) {
        AstNodeBinaryOp* bin = (AstNodeBinaryOp*)ast;
        // 2. Verify Operator is %
        CHECK(bin->op.type == TOKEN_PERCENT, "Operator is TOKEN_PERCENT");
        
        // 3. Verify Operands
        CHECK(bin->left->type == NODE_INT_LITERAL, "Left is INT");
        CHECK(bin->right->type == NODE_INT_LITERAL, "Right is INT");
    }

    free_ast(root);
}

void test_parser_compound_assignment() {
    // The Magic Trick: The parser reads "x += 5;" but produces AST for "x = x + 5;"
    const char* source = "x += 5;";
    AstNode* root = parse(source, 0);
    
    // Note: Assignments are expressions in C/Determa, but if top-level, 
    // they might be wrapped in EXPR_STMT depending on your parser logic.
    // However, your parse_assignment returns a NODE_VAR_ASSIGN directly 
    // when it consumes the '=', and parse_statement might wrap it.
    // Let's use our helper which handles unwrapping.
    AstNode* ast = get_first_stmt_expr(root);

    CHECK(ast != NULL, "AST exists");

    // 1. It should be an Assignment Node
    CHECK(ast->type == NODE_VAR_ASSIGN, "Root is VAR_ASSIGN (Desugared)");
    
    if (ast->type == NODE_VAR_ASSIGN) {
        AstNodeVarAssign* assign = (AstNodeVarAssign*)ast;
        
        // 2. Target is 'x'
        CHECK(assign->name.length == 1 && assign->name.lexeme[0] == 'x', "Target is 'x'");
        
        // 3. Value is 'x + 5' (Binary Op)
        CHECK(assign->expression->type == NODE_BINARY_OP, "Value is BINARY_OP");
        
        if (assign->expression->type == NODE_BINARY_OP) {
            AstNodeBinaryOp* op = (AstNodeBinaryOp*)assign->expression;
            
            // 4. Operator is '+' (Converted from +=)
            CHECK(op->op.type == TOKEN_PLUS, "Inner operator is PLUS");
            
            // 5. Left side is 'x' (VarAccess)
            CHECK(op->left->type == NODE_VAR_ACCESS, "Left operand is VAR_ACCESS");
            
            // 6. Right side is '5'
            CHECK(op->right->type == NODE_INT_LITERAL, "Right operand is INT 5");
        }
    }

    free_ast(root);
}