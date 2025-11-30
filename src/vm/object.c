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

// Helper to allocate memory for an object
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

static Obj* allocate_object(size_t size, ObjType type) {
    // 1. Allocate the raw memory
    Obj* object = (Obj*)malloc(size);
    if (object == NULL) {
        fprintf(stderr, "Fatal: Out of memory.\n");
        exit(1);
    }

    // 2. Initialize base state
    object->type = type;

    // 3. Add to the VM's tracking list (The "Tracker List" solution)
    // This inserts at the head of the list.
    object->next = vm.objects;
    vm.objects = object;

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
    char* heapChars = (char*)malloc(length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocate_string(heapChars, length);
}

ObjString* take_string(char* chars, int length) {
    return allocate_string(chars, length);
}

void print_object(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}