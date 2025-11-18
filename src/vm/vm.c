/**
 * @file vm.c
 * @brief Implementation of the Virtual Machine execution loop.
 */

#include <stdio.h>
#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/common.h"
#include "vm/value.h"

// The Global VM instance
VM vm;

static void reset_stack() {
    vm.stackTop = vm.stack;
}

void init_vm() {
    reset_stack();
}

void free_vm() {
    // Nothing to free yet (stack is static array)
}

// --- Stack Operations ---

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

// --- The Core Execution Loop ---

static InterpretResult run() {
    // Helper macros for reading bytecode
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    
    // Helper macro for binary operations (ensure order is correct!)
    // Pop b, Pop a, Push (a op b)
    #define BINARY_OP(op) \
        do { \
            Value b = pop(); \
            Value a = pop(); \
            push(a op b); \
        } while (false)

    for (;;) {
        
        #ifdef DEBUG_TRACE_EXECUTION
            // Debug: Print Stack
            printf("          ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[ ");
                print_value(*slot);
                printf(" ]");
            }
            printf("\n");
            
            // Debug: Disassemble instruction (simple version)
            printf("IP %04ld: Opcode %d\n", (long)(vm.ip - vm.chunk->code), *vm.ip);
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            
            case OP_NEGATE: {
                // Unary minus
                // In-place modification: pop, negate, push
                // Optimization: just negate the top of stack directly
                *(vm.stackTop - 1) = -(*(vm.stackTop - 1));
                break;
            }

            case OP_PRINT: {
                print_value(pop());
                printf("\n");
                break;
            }

            case OP_RETURN: {
                // Exit the interpreter loop
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}