/**
 * @file value.c
 * @author Andrew Fernandes
 * @brief Implementations of Value operations
 * File created to modularize the expanding value from simple int to tagged union
 * @version 0.1
 * @date 2025-11-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm/value.h"
#include "vm/chunk.h"
#include "vm/object.h"

void print_value(Value value) {
    switch (value.type) {
        case VAL_BOOL:{
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        }
        case VAL_INT:{
            printf("%d", AS_INT(value));
            break;
        }
        case VAL_OBJ: {
            print_object(value);
            break;
        }
    }
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_INT:  return AS_INT(a) == AS_INT(b);
        case VAL_OBJ: {
            // For now, simple pointer equality. 
            // Later implement deep equality for strings.
            return AS_OBJ(a) == AS_OBJ(b);
        }
        // case VAL_OBJ:
        default:       return false; // Should be unreachable
    }
}