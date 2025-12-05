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
    OP_TRUE,         // Push True
    OP_FALSE,        // Push False

    // --- Arithmetic ---
    OP_ADD,          // Pop 2, Add, Push result
    OP_SUBTRACT,     // Pop 2, Subtract, Push result
    OP_MULTIPLY,     // Pop 2, Multiply, Push result
    OP_DIVIDE,       // Pop 2, Divide, Push result
    OP_MODULO,       // Pop 2, Divide, Push remainder
    OP_NEGATE,       // Pop 1, Negate, Push result

    // --- Logic & Comparison ---
    OP_NOT,          // !
    OP_EQUAL,        // ==
    OP_GREATER,      // >
    OP_LESS,         // <

    // --- Variables ---
    OP_GET_GLOBAL,      // Operand: [1 byte index] Action: Push globals[index] onto stack
    OP_SET_GLOBAL,      // Operand: [1 byte index] Action: Pop stack, store in globals[index]

    OP_GET_LOCAL,       // Operand: [1 byte index] Action: Push value from stack[index]
    OP_SET_LOCAL,       // Operand: [1 byte index] Action: Store top of stack into stack[index] (Peek, don't pop)

    // --- Control Flow (NEW) ---
    OP_POP,           // Pop top value (cleanup)
    OP_JUMP,          // Unconditional Jump (Forward)
    OP_JUMP_IF_FALSE, // Conditional Jump (Forward)
    OP_LOOP,          // Unconditional Jump (Backward)

    // --- Statements ---
    OP_PRINT,        // Pop 1, Print it
    OP_RETURN,       // Return from script (stop execution)
} OpCode;

#endif // VM_OPCODE_H