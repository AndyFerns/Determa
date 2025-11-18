/**
 * @file compiler.h
 * @author Andrew Fernandes
 * @brief Compiler headerfile defining structs and helpers used to walk the ast and compile it into a bytecode chunk
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef VM_COMPILER_H
#define VM_COMPILER_H

#include "vm/chunk.h"
// We will include ast.h in the .c file to decouple

// Forward declaration
struct AstNode;

/**
 * @brief Compiles an AST into a bytecode Chunk.
 * @param ast The root of the AST.
 * @param chunk The chunk to write bytecode into.
 * @return 1 on success, 0 on error.
 */
int compile_ast(struct AstNode* ast, Chunk* chunk);

#endif // VM_COMPILER_H