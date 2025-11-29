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