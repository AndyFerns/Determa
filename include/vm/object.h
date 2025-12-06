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
#include "chunk.h"
#include "vm/value.h"

// Forward declarations to avoid circular dependencies
typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjFunction ObjFunction;

/**
 * @brief Identifies the specific type of an object.
 * @enum ObjType
 */
typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
} ObjType;

/**
 * @struct Obj
 * @brief The base "class" for all heap objects.
 * Contains the type tag and a pointer to the next object
 * in the global list (for memory tracking).
 */
struct Obj {
    ObjType type;
    bool isMarked;
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

typedef struct {
    Obj obj;        // Base class state
    int arity;      // Number of parameters
    Chunk chunk;    // the bytecode for This function
    ObjString* name;// Function name (for debugging)
} ObjFunction;


// --- Macros for casting ---
#define OBJ_TYPE(value)   (AS_OBJ(value)->type)
#define IS_STRING(value)  (isObjType(value, OBJ_STRING))
#define IS_FUNCTION(value)  (isObjType(value, OBJ_FUNCTION))

#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_FUNCTION(value)  ((ObjFunction*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

// --- Functions ---

/**
 * @brief Creates a new string object by copying the given C-string.
 */
ObjString* copy_string(const char* chars, int length);

/**
 * @brief Takes ownership of a string constant (for future optimization).
 */
ObjString* take_string(char* chars, int length);

/**
 * @brief Creates a new Function object by parsing the bytecode chunk
 * 
 * @return ObjFunction* 
 */
ObjFunction* new_function();

/**
 * @brief Helper for macros to check object type safely.
 */
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

/**
 * @brief Prints an object to stdout.
 */
void print_object(Value value);


#endif