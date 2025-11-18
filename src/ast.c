/**
 * @file ast.c
 * @author Andrew Fernandes
 * @brief Defines the Data structures for the Abstract Syntax Tree (AST)
 * 
 * This file includes all the function definitions and helper functions for AST parsing
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Private Helper to print with indentation ---
static void print_ast_recursive(AstNode* node, int indent) {
    if (node == NULL) {
        printf("%*sNULL_NODE\n", indent, "");
        return;
    }

    // Indent
    printf("%*s", indent, "");

    switch (node->type) {
        case NODE_PROGRAM: {
            AstNodeProgram* p = (AstNodeProgram*)node;
            printf("PROGRAM (%d statements)\n", p->statement_count);
            for (int i = 0; i < p->statement_count; i++) {
                print_ast_recursive(p->statements[i], indent + 2);
            }
            break;
        }
        case NODE_INT_LITERAL: {
            AstNodeIntLiteral* n = (AstNodeIntLiteral*)node;
            printf("INT_LITERAL: %d\n", n->value);
            break;
        }
        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)node;
            printf("BINARY_OP: %s\n", token_type_to_string(n->op.type));
            print_ast_recursive(n->left, indent + 2);
            print_ast_recursive(n->right, indent + 2);
            break;
        }
        case NODE_VAR_DECL: {
            AstNodeVarDecl* n = (AstNodeVarDecl*)node;
            printf("VAR_DECL: %.*s\n", n->name.length, n->name.lexeme);
            print_ast_recursive(n->init, indent + 2);
            break;
        }
        case NODE_VAR_ACCESS: {
            AstNodeVarAccess* n = (AstNodeVarAccess*)node;
            printf("VAR_ACCESS: %.*s\n", n->name.length, n->name.lexeme);
            break;
        }
        case NODE_PRINT_STMT: {
            AstNodePrintStmt* n = (AstNodePrintStmt*)node;
            printf("PRINT_STMT\n");
            print_ast_recursive(n->expression, indent + 2);
            break;
        }
        case NODE_EXPR_STMT: { 
            AstNodeExprStmt* n = (AstNodeExprStmt*)node;
            printf("EXPR_STMT\n");
            print_ast_recursive(n->expression, indent + 2);
            break;
        }
        // We will add more cases here
        default:
            printf("UNKNOWN_NODE\n");
            break;
    }
}

/**
 * @brief Prints an AST tree to the console (for debugging)
 */
void print_ast(AstNode* node) {
    print_ast_recursive(node, 0);
}

/**
 * @brief Frees an entire AST tree recursively
 * 
 * This performs a full post-order traversal and frees:
 *  - All child nodes
 *  - Dynamic arrays (in NODE_PROGRAM)
 *  - The node itself
 * 
 * @param AstNode* Pointer to the abstract syntax tree root node
 */
void free_ast(AstNode* node) {
    if (node == NULL) return;

    // Use post-order traversal to free
    switch (node->type) {
        case NODE_PROGRAM: {
            // Program holds a list of statements
            AstNodeProgram* prog = (AstNodeProgram*)node;

            // Free each statement
            for (int i = 0; i < prog->statement_count; i++) {
                free_ast(prog->statements[i]);
            }

            // Free the dynamic statement array
            free(prog->statements);
            break;
        }

        /* ================================
         *  INTEGER LITERAL
         *  (No children)
         * ================================ */
        // No children, just free the node
        case NODE_INT_LITERAL:
            break;

        /* ================================
         *  BINARY OPERATION
         *  e.g., left + right
         * ================================ */
        // Standard binary operation left 'op' right
        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)node;
            free_ast(n->left);
            free_ast(n->right);
            break;
        }

        /* ================================
         *  VARIABLE DECLARATION
         *  var x = init;
         * ================================ */
        case NODE_VAR_DECL: {
            AstNodeVarDecl* decl = (AstNodeVarDecl*)node;
            free_ast(decl->init);  // May be NULL
            break;
        }

        /* ================================
         *  VARIABLE ASSIGNMENT
         *  x = expr;
         * ================================ */
        case NODE_VAR_ASSIGN: {
            AstNodeVarAssign* assign = (AstNodeVarAssign*)node;
            free_ast(assign->expression);
            break;
        }

        /* ================================
         *  VARIABLE ACCESS
         *  (No children)
         * ================================ */
        case NODE_VAR_ACCESS: {
            // Token holds no heap memory → no action needed
            break;
        }

        /* ================================
         *  PRINT STATEMENT
         *  print expr;
         * ================================ */
        case NODE_PRINT_STMT: {
            AstNodePrintStmt* n = (AstNodePrintStmt*)node;
            free_ast(n->expression);
            break;
        }

        /* ================================
         *  EXPRESSION STATEMENT
         *  expr;
         * ================================ */
        case NODE_EXPR_STMT: {
            AstNodeExprStmt* est = (AstNodeExprStmt*)node;
            free_ast(est->expression);
            break;
        }

        // We will add more cases here
        default:
            // No-op
            break;
    }
    
    // Free the node itself
    free(node);
}


// =============================
// --- Constructor functions ---
// =============================


/**
 * @brief Stores a "Program" as a list of expressions
 * 
 * @param statements 
 * @param count 
 * @return AstNode* 
 */
AstNode* new_program_node(AstNode** statements, int count) {
    AstNodeProgram* node = (AstNodeProgram*)malloc(sizeof(AstNodeProgram));
    if (!node) return NULL;

    node->node.type = NODE_PROGRAM;

    // Initial dynamic array capacity
    node->capacity = (count > 0) ? count * 2 : 8;
    node->statements = (AstNode**)malloc(sizeof(AstNode*) * node->capacity);

    if (!node->statements) {
        free(node);
        return NULL;
    }

    // Copy initial statements if provided
    if (statements && count > 0) {
        memcpy(node->statements, statements, sizeof(AstNode*) * count);
        node->statement_count = count;
    } else {
        node->statement_count = 0;
    }

    return (AstNode*)node;
}

/**
 * @brief Adds a statement to the Program node
 * 
 * Helper Function for NODE_PROGRAM
 * 
 * @param program_node pointer to an AstNodeProgram (base AstNode ptr)
 * @param statement Pointer to a list of statements required to be appended
 */
void program_add_statement(AstNode* program_node, AstNode* statement) {
    if (!program_node || program_node->type != NODE_PROGRAM) return;

    AstNodeProgram* p = (AstNodeProgram*)program_node;

    // Expand array if necessary
    if (p->statement_count >= p->capacity) {
        p->capacity *= 2;
        AstNode** new_block = (AstNode**)realloc(
            p->statements,
            sizeof(AstNode*) * p->capacity
        );

        if (!new_block) {
            // realloc failed → KEEP OLD BLOCK, DO NOT MODIFY POINTER
            return;
        }
        p->statements = new_block;
    }

    p->statements[p->statement_count++] = statement;
}

/**
 * @brief Creates a new Variable Declaration AST node.
 * 
 * @param name 
 * @param initializer 
 * @return AstNode* 
 */
AstNode* new_var_decl_node(Token name, AstNode* initializer) {
    AstNodeVarDecl* node = (AstNodeVarDecl*)malloc(sizeof(AstNodeVarDecl));
    if (!node) return NULL;
    node->node.type = NODE_VAR_DECL;
    node->name = name;
    node->init = initializer;
    return (AstNode*)node;
}

/**
 * @brief Creates a new variable access AST Node
 * 
 * @param name 
 * @return AstNode* 
 */
AstNode* new_var_access_node(Token name) {
    AstNodeVarAccess* node = (AstNodeVarAccess*)malloc(sizeof(AstNodeVarAccess));
    if (!node) return NULL;
    node->node.type = NODE_VAR_ACCESS;
    node->name = name;
    return (AstNode*)node;
}

/**
 * @brief Creates a new Print statement AST Node
 * 
 * @param expression expression node whose value should be printed
 * @return AstNode* pointer to the newly allocated AstNode, or NULL on allocation failure
 */
AstNode* new_print_stmt_node(AstNode* expression) {
    AstNodePrintStmt* node = (AstNodePrintStmt*)malloc(sizeof(AstNodePrintStmt));
    if (!node) return NULL;
    node->node.type = NODE_PRINT_STMT;
    node->expression = expression;
    return (AstNode*)node;
}

/**
 * @brief Creates a new Integer Literal AST node
 */
AstNode* new_int_literal_node(int value) {
    AstNodeIntLiteral* node = (AstNodeIntLiteral*)malloc(sizeof(AstNodeIntLiteral));
    if (node == NULL) return NULL;
    
    node->node.type = NODE_INT_LITERAL;
    node->value = value;
    return (AstNode*)node;
}

AstNode* new_expr_stmt_node(AstNode* expression) {
    // if (!expression) return NULL;  // defensive check

    AstNodeExprStmt* node = (AstNodeExprStmt*)malloc(sizeof(AstNodeExprStmt));
    if (!node) return NULL;

    node->node.type = NODE_EXPR_STMT;
    node->expression = expression;

    return (AstNode*)node;
}


/**
 * @brief Creates a new Binary Operator AST node
 */
AstNode* new_binary_op_node(Token op, AstNode* left, AstNode* right) {
    AstNodeBinaryOp* node = (AstNodeBinaryOp*)malloc(sizeof(AstNodeBinaryOp));
    if (node == NULL) return NULL;

    node->node.type = NODE_BINARY_OP;
    node->op = op;
    node->left = left;
    node->right = right;
    return (AstNode*)node;
}