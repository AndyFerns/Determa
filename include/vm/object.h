/**
 * @file object.h
 * @author Andrew Fernandes
 * @brief Defines heap-allocated objects for the VM.
 * @version 0.1
 * @date 2025-11-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"

// Forward declarations to avoid circular dependencies
typedef struct Obj Obj;
typedef struct ObjString ObjString;


/**
 * @brief Identifies the specific type of an object.
 * @enum ObjType
 */
typedef enum {
    OBJ_STRING,
} ObjType;

/**
 * @struct Obj
 * @brief The base "class" for all heap objects.
 * Contains the type tag and a pointer to the next object
 * in the global list (for memory tracking).
 */
struct Obj {
    ObjType type;
    struct Obj* next; // Linked list for tracking/GC
};

/**
 * @struct ObjString
 * @brief A String object. Inherits from Obj.
 */
struct ObjString {
    Obj obj;      // Base class state
    int length;
    char* chars;  // Null-terminated C string
};

#endif