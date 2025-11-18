/**
 * @file opcode.h
 * @brief Defines the instruction set (Bytecode) for the Determa VM.
 */

#ifndef VM_OPCODE_H
#define VM_OPCODE_H

#include <stdint.h>

/**
 * @enum OpCode
 * @brief 1-byte instructions that the VM executes.
 */
typedef enum {
    // --- Loading Values ---
    OP_CONSTANT,     // Load a constant from the pool. [OP_CONSTANT] [INDEX]

    // --- Arithmetic ---
    OP_ADD,          // Pop 2, Add, Push result
    OP_SUBTRACT,     // Pop 2, Subtract, Push result
    OP_MULTIPLY,     // Pop 2, Multiply, Push result
    OP_DIVIDE,       // Pop 2, Divide, Push result
    OP_NEGATE,       // Pop 1, Negate, Push result

    // --- Statements ---
    OP_PRINT,        // Pop 1, Print it
    OP_RETURN,       // Return from script (stop execution)
} OpCode;

#endif // VM_OPCODE_H