/**
 * @file value.h
 * @brief Defines how data is represented at runtime in the VM.
 * Implements a Tagged Union to support Ints, Bools, and Objects.
 */

#ifndef VM_VALUE_H
#define VM_VALUE_H

#include "common.h"

// Forward declaration of Obj (defined in object.h)
typedef struct Obj Obj;

/**
 * @enum Value Type It stores the total types of values identifiable by the language
 * @brief Identifies the type of data stored in a Value struct.
 * 
 */
typedef enum {
    VAL_BOOL,
    VAL_INT,
    VAL_OBJ
} ValueType;


typedef struct {
    ValueType type;
    union {
        bool boolean;
        int integer;
        Obj* obj; // TODO
    } as;
} Value;

// --- Macros for Type Checking ---
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_INT(value)     ((value).type == VAL_INT)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

// --- Macros for Unwrapping Values (Unsafe - Check type first!) ---
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_INT(value)     ((value).as.integer)
#define AS_OBJ(value)     ((value).as.obj)

// --- Macros for Creating Values ---
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define INT_VAL(value)    ((Value){VAL_INT, {.integer = value}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, {.obj = (Obj*)object}})


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

/**
 * @brief Checks if two values are equal.
 */
bool values_equal(Value a, Value b);

#endif // VM_VALUE_H