/**
 * @file chunk.h
 * @brief Defines the 'Chunk' data structure.
 * * A Chunk is a sequence of bytecode instructions and a pool of constants.
 * It is the low-level representation of a program.
 */

#ifndef VM_CHUNK_H
#define VM_CHUNK_H

#include "common.h"
#include "value.h"

/**
 * @struct Chunk
 * @brief A dynamic array of bytecode instructions.
 */
typedef struct {
    int count;              // Number of bytes currently in use
    int capacity;           // Allocated capacity
    uint8_t* code;          // Pointer to the bytecode stream
    int* lines;             // Array storing line numbers matching the bytecode (for debugging)
    ValueArray constants;   // Pool of constants (numbers, strings) used in this chunk
} Chunk;

/**
 * @brief Initializes a new, empty chunk.
 */
void init_chunk(Chunk* chunk);

/**
 * @brief Frees all memory associated with the chunk.
 */
void free_chunk(Chunk* chunk);

/**
 * @brief Appends a byte (opcode or operand) to the chunk.
 * @param chunk The chunk to write to.
 * @param byte The byte to write.
 * @param line The line number in the source code (for error reporting).
 */
void write_chunk(Chunk* chunk, uint8_t byte, int line);

/**
 * @brief Adds a constant to the chunk's constant pool.
 * @return The index of the constant in the pool (to be used with OP_CONSTANT).
 */
int add_constant(Chunk* chunk, Value value);

#endif // VM_CHUNK_H