/**
 * @file value.h
 * @brief Defines how data is represented at runtime in the VM.
 */

#ifndef VM_VALUE_H
#define VM_VALUE_H

#include "common.h"

/**
 * @brief The core data type for the VM.
 * Currently just an integer, but will become a tagged union in Phase 5.
 */
typedef int Value;

/**
 * @struct ValueArray
 * @brief A dynamic array to store constants (like numbers/strings) found in the code.
 */
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

// --- ValueArray Helpers ---
void init_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);
void print_value(Value value);

#endif // VM_VALUE_H