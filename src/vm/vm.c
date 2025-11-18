/**
 * @file vm.c
 * @brief Implementation of the Virtual Machine execution loop.
 *
 * This module contains stack manipulation utilities, the main interpreter loop,
 * and helper macros used for bytecode execution. The VM is a simple stack-based
 * registerless interpreter similar to Lox/Wren-style VMs.
 */

#include <stdio.h>
#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/common.h"
#include "vm/value.h"

// The global VM instance (single VM model)
VM vm;

/**
 * @brief Reset the VM operand stack to an empty state.
 */
static void reset_stack() {
    vm.stackTop = vm.stack;
}

/**
 * @brief Initialize the VM (reset stack, clear state).
 */
void init_vm() {
    reset_stack();
}

/**
 * @brief Free VM resources.
 *
 * Currently unused because the VM uses static arrays,
 * but remains for future heap-managed values.
 */
void free_vm() {
    // No dynamic allocations yet.
}

// -----------------------------------------------------------------------------
// Stack Operations
// -----------------------------------------------------------------------------

/**
 * @brief Push a value onto the VM stack.
 *
 * @param value The value to push.
 */
void push(Value value) {
    // Optimization: optional debug safety check
    // Ensures we do not overflow stack
    // (no behavior change unless the stack overflows)
#ifdef VM_STACK_CHECK
    if (vm.stackTop - vm.stack >= STACK_MAX) {
        fprintf(stderr, "Stack overflow!\n");
        return;
    }
#endif
    *vm.stackTop++ = value;
}

/**
 * @brief Pop the top value off the stack.
 *
 * @return The popped stack value.
 */
Value pop() {
    // Optimization: Pre-decrement is slightly faster and idiomatic
    return *--vm.stackTop;
}

/**
 * @brief Peek at a value on the stack without removing it.
 *
 * @param distance How far from the top: 0 = top, 1 = below top, etc.
 * @return The stack value at that distance.
 */
Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief Execute bytecode instructions until OP_RETURN.
 *
 * @return InterpretResult The final interpreter state.
 *
 * Performance note:
 * The interpreter loop caches frequently-used VM fields (stackTop, ip)
 * into local variables to make the loop tighter. This is a common VM
 * optimization pattern.
 */
static InterpretResult run() {
    // Optimization: local cached register copies for speed
    uint8_t* ip = vm.ip;
    Value* stackTop = vm.stackTop;
    Value* stack = vm.stack;

    // Cache constants array pointer for faster access
    Value* constants = vm.chunk->constants.values;

    // Read macros using local cached registers
    #define READ_BYTE() (*ip++)
    #define READ_CONSTANT() (constants[READ_BYTE()])

    // Optimized binary operation macro
    // Note: using const Value ensures no aliasing surprises & better optimization
    #define BINARY_OP(op)                          \
        do {                                       \
            const Value b = *(--stackTop);         /* pop b */ \
            const Value a = *(--stackTop);         /* pop a */ \
            *stackTop++ = (a op b);                /* push (a op b) */ \
        } while (false)

    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = stack; slot < stackTop; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");

        printf("IP %04ld: Opcode %d\n",
               (long)(ip - vm.chunk->code),
               *ip);
#endif

        uint8_t instruction = READ_BYTE();

        switch (instruction) {

            case OP_CONSTANT: {
                *stackTop++ = READ_CONSTANT();  // Faster than push()
                break;
            }

            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;

            case OP_NEGATE: {
                // Optimized negate (modify top-of-stack in place)
                stackTop[-1] = -stackTop[-1];
                break;
            }

            case OP_PRINT: {
                Value v = *(--stackTop);   // pop()
                print_value(v);
                printf("\n");
                break;
            }

            case OP_RETURN: {
                // Sync cached registers and exit
                vm.ip = ip;
                vm.stackTop = stackTop;
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

/**
 * @brief Load a bytecode chunk into the VM and begin execution.
 *
 * @param chunk Pointer to a compiled Chunk of bytecode instructions.
 * @return InterpretResult OK or runtime error state.
 */
InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}
