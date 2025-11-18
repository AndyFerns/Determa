/**
 * @file symbol.c
 * @author Andrew Fernandes
 * @brief Implementation of the Symbol Table (hash map or list)
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "types.h"
#include "symbol.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void symbol_table_init(SymbolTable* table) {
    table->capacity = 16;
    table->count = 0;
    table->current_depth = 0;
    table->symbols = (Symbol*)malloc(sizeof(Symbol) * table->capacity);
}

void symbol_table_free(SymbolTable* table) {
    free(table->symbols);
    table->count = 0;
}

void symbol_table_enter_scope(SymbolTable* table) {
    table->current_depth++;
}

void symbol_table_exit_scope(SymbolTable* table) {
    // Remove all symbols that belong to the current depth
    while (table->count > 0 && 
           table->symbols[table->count - 1].depth == table->current_depth) {
        table->count--;
    }
    table->current_depth--;
}