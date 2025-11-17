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

/**
 * @brief Frees an entire AST tree recursively
 *
 * (STUB) We will implement this later
 */
void free_ast(AstNode* node) {
    if (node == NULL) return;
    // TODO: Implement recursive free
}

/**
 * @brief Prints an AST tree to the console (for debugging)
 *
 * (STUB) We will implement this later
 */
void print_ast(AstNode* node) {
    if (node == NULL) {
        printf("NULL_NODE\n");
        return;
    }
    // TODO: Implement recursive print
}