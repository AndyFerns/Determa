/**
 * @file object.c
 * @author Andrew Fernandes
 * @brief Implementation of object management
 * @version 0.1
 * @date 2025-11-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vm/object.h"
#include "vm/value.h"
#include "vm/vm.h"
#include "vm/memory.h" 


// Macro to allocate memory using the GC tracker
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))


// Helper to allocate memory for an object
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)



static Obj* allocate_object(size_t size, ObjType type) {
    // Use the GC-aware reallocate to get memory
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    
    object->type = type;
    object->isMarked = false; // --- NEW: Initialize mark

    object->next = vm.objects;
    vm.objects = object;

    #ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
    #endif

    return object;
}

/**
 * @brief Function to allocate memory from the heap for string characters
 * 
 * @param chars 
 * @param length 
 * @return ObjString* 
 */
static ObjString* allocate_string(char* chars, int length) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString* copy_string(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length + 1);
    
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocate_string(heapChars, length);
}

ObjString* take_string(char* chars, int length) {
    return allocate_string(chars, length);
}

/**
 * @brief Helper function to deal with string concatenation
 * 
 * @param a First string to be concatenated
 * @param b Second String to be concatenated
 * @return ObjString* 
 */
ObjString* concatenate(ObjString* a, ObjString* b) {
    int length = a->length + b->length;

    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    return take_string(chars, length);
}


ObjFunction* new_function() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL; // NULL name means top-level script
    init_chunk(&function->chunk);
    return function;
}

void print_object(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;

        case OBJ_FUNCTION: {
            ObjFunction* fn = AS_FUNCTION(value);
            if (fn->name == NULL) {
                printf("<script>");
            } else {
                printf("<fn %s>", fn->name->chars);
            }
            break;
        }
    }
}