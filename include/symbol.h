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