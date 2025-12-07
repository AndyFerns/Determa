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
#include "vm/object.h"

// Forward declaration
struct AstNode;


/**
 * @brief Initializes/Resets the persistent state of the compiler (symbol table).
 * @return voic (its just an initializer)
 */
void init_compiler();

/**
 * @brief Compiles an AST into a bytecode Chunk.
 * @param ast The root of the AST.
 * @param chunk The chunk to write bytecode into.
 * @return 1 on success, 0 on error.
 */
ObjFunction* compile_ast(struct AstNode* ast);

/**
 * @brief Compiles source code into a Function Object.
 * Handles parsing internally.
 * @param source The source code string.
 * @return A new ObjFunction containing the bytecode, or NULL on error.
 */
ObjFunction* compile(const char* source);

/**
 * @brief Function to mark compiler roots and use it in garbage collection and garbage deletion
 */
void mark_compiler_roots();

/**
 * @brief Function to free all globals once compilation is done
 * call on shutdown or before resetting the compiler
 */
void free_global_symbols();

#endif // VM_COMPILER_H