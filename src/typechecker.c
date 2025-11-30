/**
 * @file typechecker.c
 * @author Andrew Fernandes
 * @brief Recursive AST type-checker with scope validation.
 *
 * The type checker walks the AST, infers expression types, validates
 * variable usage, and ensures type consistency in expressions and statements.
 *
 * It uses a SymbolTable internally to validate variable declarations and
 * ensure proper scoping rules.
 *
 * @version 0.1
 * @date 2025-11-18
 */

#include "typechecker.h"
#include "symbol.h"
#include "ast.h"

#include <stdio.h>

// --- Persistent State ---
static SymbolTable globalSymbols;
static int is_initialized = 0;

/**
 * @struct TypeChecker
 * @brief Internal state for the recursive type-checking pass.
 *
 * @var TypeChecker::symbols
 * Symbol table used while traversing the AST.
 *
 * @var TypeChecker::had_error
 * Set to 1 if any type error is reported.
 */
typedef struct {
    SymbolTable symbols;
    int had_error;
} TypeChecker;


void init_typechecker() {
    if (!is_initialized) {
        symbol_table_init(&globalSymbols);
    }
    
    is_initialized = 1;
}

void free_typechecker() {
    if (is_initialized) {
        symbol_table_free(&globalSymbols);
        is_initialized = 0;
    }
}


/**
 * @brief Report a type error and mark the checker as invalid.
 *
 * @param tc Pointer to the active type-checker instance.
 * @param msg Human-readable error string.
 */
static void error(TypeChecker* tc, const char* msg) {
    printf("Type Error: %s\n", msg);
    tc->had_error = 1;
}

// Forward declarations
static DataType check_expression(TypeChecker* tc, AstNode* expr);
static void     check_statement(TypeChecker* tc, AstNode* stmt);

/**
 * @brief Recursively checks the type of an expression AST node.
 *
 * Supported:
 *  - Integer literals
 *  - Variable access
 *  - Binary operations (+ only for now)
 *
 * @param tc   Active type-checker context.
 * @param expr Expression AST node.
 * @return DataType  Resulting type, or TYPE_ERROR if invalid.
 */
static DataType check_expression(TypeChecker* tc, AstNode* expr) {
    if (expr == NULL)
        return TYPE_VOID;

    switch (expr->type) {

        case NODE_INT_LITERAL:
            return TYPE_INT;

        case NODE_STRING_LITERAL: 
            return TYPE_STRING;

        case NODE_UNARY_OP: {
            AstNodeUnaryOp* n = (AstNodeUnaryOp*)expr;
            DataType type = check_expression(tc, n->operand);
            if (type != TYPE_INT) {
                error(tc, "Unary '-' only applies to numbers.");
                return TYPE_ERROR;
            }
            return TYPE_INT;
        }

        case NODE_VAR_ACCESS: {
            AstNodeVarAccess* access = (AstNodeVarAccess*)expr;
            // Use tc->symbols, not globalSymbols directly
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

            // Optimized: Only call once per child expression
            DataType leftType  = check_expression(tc, op->left);
            DataType rightType = check_expression(tc, op->right);

            // Propagate type errors
            if (leftType == TYPE_ERROR || rightType == TYPE_ERROR)
                return TYPE_ERROR;


            // INT + INT 
            if (leftType == TYPE_INT && rightType == TYPE_INT)
                return TYPE_INT;

            // STRING + STRING (string concatenation)
            if (op->op.type == TOKEN_PLUS) {
                if (leftType == TYPE_STRING && rightType == TYPE_STRING) 
                    return TYPE_STRING;
            }

            error(tc, "Type mismatch. Operations support INT (+-*/) or STRING (+ only).");
            return TYPE_ERROR;
        }

        default:
            // Unknown expression node
            return TYPE_ERROR;
    }
}

/**
 * @brief Recursively type-check a statement node.
 *
 * Supported:
 *  - Variable declarations
 *  - Print statements
 *  - Expression statements
 *  - Program blocks (list of statements)
 *
 * @param tc   Active type-checker context.
 * @param stmt Statement AST node.
 */
static void check_statement(TypeChecker* tc, AstNode* stmt) {
    if (stmt == NULL)
        return;

    switch (stmt->type) {

        case NODE_VAR_DECL: {
            AstNodeVarDecl* decl = (AstNodeVarDecl*)stmt;
            DataType initType = TYPE_VOID;

            // Determine type from initializer
            if (decl->init != NULL) {
                initType = check_expression(tc, decl->init);
                if (initType == TYPE_ERROR) return;
            } else {
                error(tc, "Variable declaration requires an initializer for type inference.");
                return;
            }

            if (initType == TYPE_ERROR)
                return;

            // FIX: Use tc->symbols
            int defined = symbol_table_define(&tc->symbols, decl->name.lexeme, decl->name.length, initType);

            // Add to symbol table
            // If failed (and not global), report error. 
            // Global redefinition is handled inside symbol_table_define returning 1.
            if (!defined && tc->symbols.current_depth > 0) {
                 char buf[256];
                 snprintf(buf, sizeof(buf), "Variable '%.*s' already declared in this scope", decl->name.length, decl->name.lexeme);
                 error(tc, buf);
            }
            break;
            break;
        }

        case NODE_PRINT_STMT: {
            AstNodePrintStmt* print = (AstNodePrintStmt*)stmt;
            DataType type = check_expression(tc, print->expression);

            if (type == TYPE_ERROR)
                return;

            if (type == TYPE_VOID)
                error(tc, "Cannot print a void expression.");

            break;
        }

        case NODE_EXPR_STMT: {
            AstNodeExprStmt* exprStmt = (AstNodeExprStmt*)stmt;
            check_expression(tc, exprStmt->expression);
            break;
        }

        case NODE_PROGRAM: {
            AstNodeProgram* prog = (AstNodeProgram*)stmt;

            for (int i = 0; i < prog->statement_count; i++)
                check_statement(tc, prog->statements[i]);

            break;
        }

        default:
            // Unknown or unsupported statement type
            break;
    }
}

/**
 * @brief Entry point for the complete type-checking pass.
 *
 * Initializes a type-checking context, runs the recursive walker over the AST,
 * and validates all statements and expressions.
 *
 * @param root The root AST node (typically NODE_PROGRAM).
 * @return int 1 if type checking passed, 0 if any error occurred.
 */
int typecheck_ast(AstNode* root) {
    if (!is_initialized) init_typechecker(); // Lazy init safety

    TypeChecker tc;
    tc.had_error = 0;

    // Copy global → local
    tc.symbols = globalSymbols;

    // Local scope (for this AST run)
    // symbol_table_enter_scope(&tc.symbols);

    // symbol_table_init(&globalSymbols);

    // A program node represents a list of statements
    if (root->type == NODE_PROGRAM) {
        AstNodeProgram* prog = (AstNodeProgram*)root;
        for (int i = 0; i < prog->statement_count; i++)
            check_statement(&tc, prog->statements[i]);
    } else {
        // For tests or single-node checking
        check_statement(&tc, root);
    }

    // Pop temporary scope
    symbol_table_exit_scope(&tc.symbols);

    // If no error, commit back to persistent global table
    if (!tc.had_error)
        globalSymbols = tc.symbols;

    // symbol_table_free(&globalSymbols);
    return !tc.had_error;
}
