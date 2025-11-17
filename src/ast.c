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

// --- Private Helper to print with indentation ---
static void print_ast_recursive(AstNode* node, int indent) {
    if (node == NULL) {
        printf("%*sNULL_NODE\n", indent, "");
        return;
    }

    // Indent
    printf("%*s", indent, "");

    switch (node->type) {
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
 */
void free_ast(AstNode* node) {
    if (node == NULL) return;

    // Use post-order traversal to free
    switch (node->type) {
        case NODE_INT_LITERAL:
            // No children, just free the node
            break;
        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)node;
            free_ast(n->left);
            free_ast(n->right);
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

// --- Constructor Functions ---

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