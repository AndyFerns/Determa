/**
 * @file types.h
 * @author Andrew Fernandes
 * @brief Defines DataType enum and helpers.
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

/**
 * @enum DataType
 * 
 * @brief Primitive types supported by the language 
 * 
 */
typedef enum {
    TYPE_VOID,      // For statements which dont return a value
    TYPE_INT,       // Integer values
    TYPE_BOOL,      // Boolean falues (TODO)
    TYPE_STRING,    // String (TODO)
    TYPE_ERROR      // Used when type checking fails
} DataType;

/**
 * @brief Converts a DataType to a string for printing/debugging
 * 
 * @param type Expecting a DataType enum object as a parameter
 * @return const char* 
 */
static inline const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_VOID:   return "void";
        case TYPE_INT:    return "int";
        case TYPE_BOOL:   return "bool";
        case TYPE_STRING: return "string";
        case TYPE_ERROR:  return "<type_error>";
        default:          return "unknown";
    }
}
#endif