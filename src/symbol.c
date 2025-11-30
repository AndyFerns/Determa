/**
 * @file symbol.c
 * @author Andrew Fernandes
 * @brief Implementation of a linear-scan symbol table with scope support.
 *
 * The symbol table stores symbols in a dynamically resized array, preserving
 * insertion order. Scopes are managed by tracking the current depth and
 * removing symbols belonging to exiting scopes.
 *
 * @version 0.1
 * @date 2025-11-18
 */

#include "types.h"
#include "symbol.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper to duplicate a string
static char* copy_string(const char* chars, int length) {
    char* string = (char*)malloc(length + 1);
    if (string == NULL) {
        fprintf(stderr, "Not enough memory for symbol name.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(string, chars, length);
    string[length] = '\0';
    return string;
}

/**
 * @brief Initialize a symbol table.
 *
 * Allocates an initial buffer of 16 symbols and resets counters.
 *
 * @param table Pointer to the SymbolTable instance to initialize.
 */
void symbol_table_init(SymbolTable* table) {
    table->capacity = 16;
    table->count = 0;
    table->current_depth = 0;

    table->symbols = (Symbol*)malloc(sizeof(Symbol) * table->capacity);
    if (!table->symbols) {
        fprintf(stderr, "Fatal: failed to allocate symbol table.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Free the memory used by the symbol table.
 *
 * memory owned externally (source buffer or AST strings).
 * Frees symbol names (because we allocate them in define()).
 *
 * @param table Pointer to the SymbolTable to free.
 */
void symbol_table_free(SymbolTable* table) {
    // Free the allocated name strings
    for (int i = 0; i < table->count; i++) {
        free((void*)table->symbols[i].name);
    }
    free(table->symbols);

    table->symbols = NULL;
    table->count = 0;
    table->capacity = 0;
    table->current_depth = 0;
}

/**
 * @brief Enter a new scope depth.
 *
 * All symbols subsequently defined will inherit this new depth,
 * until a matching exit occurs.
 *
 * @param table Pointer to the symbol table.
 */
void symbol_table_enter_scope(SymbolTable* table) {
    table->current_depth++;
}

/**
 * @brief Exit the current scope, removing symbols defined inside it.
 *
 * All symbols whose depth matches the current depth are removed.
 *
 * @param table Pointer to the symbol table.
 */
void symbol_table_exit_scope(SymbolTable* table) {
    while (table->count > 0 && table->symbols[table->count - 1].depth == table->current_depth) {

        table->count--;
        free((void*)table->symbols[table->count].name); // Free the name
    }

    if (table->current_depth > 0)
        table->current_depth--;
}

/**
 * @brief Define a new symbol in the current scope.
 *
 * Checks for redefinition within the same depth. Shadowing outer-scope
 * variables is allowed.
 *
 * @param table Pointer to the symbol table.
 * @param name Pointer to the symbol's name (not copied!).
 * @param len Length of the symbol's name.
 * @param type The data type of the symbol.
 * @return int 1 if definition succeeded, 0 if already defined in this scope.
 */
int symbol_table_define(SymbolTable* table, const char* name, int len, DataType type) {
    // Check for redefinition within this scope only
    for (int i = table->count - 1; i >= 0; i--) {
        Symbol* s = &table->symbols[i];

        // Stop once we hit an outer scope
        if (s->depth < table->current_depth) break;

        if (s->depth == table->current_depth &&
            s->name_len == len &&
            strncmp(s->name, name, len) == 0) {

            // Allow redefinition in Global Scope (Depth 0) for REPL convenience
            if (table->current_depth == 0) {
                s->type = type;
                return 1; // Success (overwrite)
            }
            return 0; // Redefinition error in same scope
        }
    }

    // Resize if needed
    if (table->count >= table->capacity) {
        size_t new_cap = table->capacity * 2;
        Symbol* new_buf = (Symbol*)realloc(table->symbols, sizeof(Symbol) * new_cap);
        if (!new_buf) {
            fprintf(stderr, "Fatal: realloc failed in symbol_table_define().\n");
            exit(EXIT_FAILURE);
        }
        table->symbols = new_buf;
        table->capacity = new_cap;
    }

    // Insert new symbol
    Symbol* s = &table->symbols[table->count++];
    s->name = copy_string(name, len);       // Assumes caller-owned lifetime
    s->name_len = len;
    s->type = type;
    s->depth = table->current_depth;

    return 1;
}

/**
 * @brief Look up a symbol by name in any accessible scope.
 *
 * Searches from innermost scope outward. Returns the most recent
 * matching symbol.
 *
 * @param table Pointer to the symbol table.
 * @param name Name to search for.
 * @param len Length of the identifier.
 * @return DataType The type of the symbol, or TYPE_ERROR if not found.
 */
DataType symbol_table_lookup(SymbolTable* table, const char* name, int len) {
    for (int i = table->count - 1; i >= 0; i--) {
        Symbol* s = &table->symbols[i];
        if (s->name_len == len && strncmp(s->name, name, len) == 0) {
            return s->type;
        }
    }
    return TYPE_ERROR;
}
