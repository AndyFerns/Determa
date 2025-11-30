/**
 * @file memory.h
 * @author Andrew Fernandes
 * @brief Garbage Collection and Memory Management.
 * @version 0.1
 * @date 2025-11-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef VM_MEMORY_H
#define VM_MEMORY_H

#include "common.h"
#include "object.h"

/**
 * @brief Reallocates memory using the GC's tracking.
 * @param pointer Existing pointer (or NULL for new allocation)
 * @param oldSize Size of the old block
 * @param newSize Size of the new block (0 to free)
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

/**
 * @brief Marks an object as reachable (Black/Gray).
 */
void mark_object(Obj* object);

/**
 * @brief Marks a Value if it contains an object.
 */
void mark_value(Value value);

/**
 * @brief Runs a full garbage collection cycle.
 */
void collect_garbage();

/**
 * @brief Frees a single object (internal use).
 */
void free_object(Obj* object);

#endif