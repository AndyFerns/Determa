/**
 * @file symbol.h
 * @author Andrew Fernandes
 * @brief Defines the Symbol Table structure.
 * 
 * Implements a stack-based symbol table to handle scopes.
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "types.h"

/**
 * @struct Symbol
 * @brief Represents a single variable in the table
 */
typedef struct {
    const char* name;  // Variable name
    int name_len;      // Length of name
    DataType type;     // The type of the variable
    int depth;         // Scope depth (0 = global, 1 = function/block, etc.)
} Symbol;

/**
 * @struct SymbolTable
 * @brief The main table structure
 */
typedef struct {
    Symbol* symbols;   // Dynamic array of symbols
    int count;         // Current number of symbols
    int capacity;      // Allocated capacity
    int current_depth; // Current scope depth
} SymbolTable;

// ================
// Helper Functions
// ================


/**
 * @brief Initializes a new symbol table
 */
void symbol_table_init(SymbolTable* table);

/**
 * @brief Frees memory used by the symbol table
 */
void symbol_table_free(SymbolTable* table);

/**
 * @brief Enters a new scope (increments depth)
 */
void symbol_table_enter_scope(SymbolTable* table);

/**
 * @brief Exits the current scope (decrements depth and removes symbols)
 */
void symbol_table_exit_scope(SymbolTable* table);

/**
 * @brief Defines a new variable in the current scope
 * @return 1 on success, 0 if variable already exists in *current* scope
 */
int symbol_table_define(SymbolTable* table, const char* name, int len, DataType type);

/**
 * @brief Looks up a variable by name (searching from innermost scope out)
 * @return The DataType of the variable, or TYPE_ERROR if not found
 */
DataType symbol_table_lookup(SymbolTable* table, const char* name, int len);

#endif // SYMBOL_H