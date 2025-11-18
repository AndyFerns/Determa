/**
 * @file typechecker.c
 * @author Andrew Fernandes
 * @brief The recursive AST walker that validates logic.
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "typechecker.h"
#include "symbol.h"
#include "ast.h"

#include <stdio.h>

// State for the type checker
typedef struct {
    SymbolTable symbols;
    int had_error;
} TypeChecker;

static void error(TypeChecker* tc, const char* msg) {
    printf("Type Error: %s\n", msg);
    tc->had_error = 1;
}

// Forward declarations
static DataType check_expression(TypeChecker* tc, AstNode* expr);
static void check_statement(TypeChecker* tc, AstNode* stmt);

static DataType check_expression(TypeChecker* tc, AstNode* expr) {
    if (expr == NULL) return TYPE_VOID;

    switch (expr->type) {
        case NODE_INT_LITERAL:
            return TYPE_INT;

        case NODE_VAR_ACCESS: {
            AstNodeVarAccess* access = (AstNodeVarAccess*)expr;
            DataType type = symbol_table_lookup(&tc->symbols, access->name.lexeme, access->name.length);
            if (type == TYPE_ERROR) {
                char buf[256];
                snprintf(buf, sizeof(buf), "Undefined variable '%.*s'", access->name.length, access->name.lexeme);
                error(tc, buf);
            }
            return type;
        }

        case NODE_BINARY_OP: {
            AstNodeBinaryOp* op = (AstNodeBinaryOp*)expr;
            DataType leftType = check_expression(tc, op->left);
            DataType rightType = check_expression(tc, op->right);

            // Simple rule: Only INT + INT is allowed for now
            if (leftType == TYPE_INT && rightType == TYPE_INT) {
                return TYPE_INT;
            }
            
            // If we already hit an error in children, propagate error
            if (leftType == TYPE_ERROR || rightType == TYPE_ERROR) return TYPE_ERROR;

            error(tc, "Type mismatch in binary operation. Expected INTs.");
            return TYPE_ERROR;
        }
        
        default:
            return TYPE_ERROR;
    }
}