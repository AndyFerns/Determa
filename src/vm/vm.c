/**
 * @file vm.c
 * @brief Implementation of the Virtual Machine execution loop.
 *
 * This module contains stack manipulation utilities, the main interpreter loop,
 * and helper macros used for bytecode execution. The VM is a simple stack-based
 * registerless interpreter similar to Lox/Wren-style VMs.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/common.h"
#include "vm/value.h"
#include "vm/object.h" // Need this to access ObjString struct for freeing
#include "vm/memory.h"

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
    vm.objects = NULL; // Initialize the tracker list

    // --- NEW: GC Init ---
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024; // Start GC at 1MB
}

// --- Helper to free a single object ---
// static void free_object(Obj* object) {
//     switch (object->type) {
//         case OBJ_STRING: {
//             ObjString* string = (ObjString*)object;
//             free(string->chars); // Free the character array
//             free(string);        // Free the struct itself
//             break;
//         }
//     }
// }

/**
 * @brief Free VM resources for heap-managed resources
 */
void free_vm() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        free_object(object); // Calls memory.c's free_object
        object = next;
    }
    vm.objects = NULL;

    // Free the gray stack
    free(vm.grayStack);
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

// --- Runtime Error Helper ---

/**
 * @brief Prints out the error message to the terminal during function runtime
 * 
 * @param format 
 */
static void runtimeError(const char* format) {
    fprintf(stderr, "%s\n", format);
    reset_stack();
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

    (void)stack;

    // Cache constants array pointer for faster access
    Value* constants = vm.chunk->constants.values;

    // Read macros using local cached registers
    #define READ_BYTE() (*ip++)
    #define READ_CONSTANT() (constants[READ_BYTE()])

    // PEEK macro to use local stackTop
    #define PEEK(distance) (stackTop[-1 - distance])

    // Optimized binary operation macro
    // Note: using const Value ensures no aliasing surprises & better optimization
    #define BINARY_OP(op) \
        do { \
            if (!IS_INT(stackTop[-1]) || !IS_INT(stackTop[-2])) { \
                vm.ip = ip; \
                vm.stackTop = stackTop; \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            int b = AS_INT(*(--stackTop)); \
            int a = AS_INT(*(--stackTop)); \
            *stackTop++ = INT_VAL(a op b); \
        } while (0)


    for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = stack; slot < stackTop; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");

        printf("IP %04ld: Opcode %d\n", (long)(ip - vm.chunk->code), *ip);
#endif

        uint8_t instruction = READ_BYTE();

        switch (instruction) {

            case OP_CONSTANT: {
                *stackTop++ = READ_CONSTANT();  // Faster than push()
                break;
            }

            // Global variable logic
            case OP_GET_GLOBAL: {
                uint8_t index = READ_BYTE();
                // Direct array access: O(1) speed
                Value val = vm.globals[index];
                *stackTop++ = val;
                break;
            }

            case OP_SET_GLOBAL: {
                uint8_t index = READ_BYTE();
                // In C: x = 10 evaluates to 10. 
                // We peek the value so it stays on stack (for expressions like a = b = 10)
                // But for simple VarDecl statements, the compiler will emit POP later.
                // For our current simple compiler, let's POP it to be safe for statements.
                Value val = *(--stackTop);
                vm.globals[index] = val;
                break;
            }

            case OP_ADD: {
                // Use PEEK macro instead of peek() function
                if (IS_STRING(PEEK(0)) && IS_STRING(PEEK(1))) {
                    // String Concatenation
                    ObjString* b = AS_STRING(PEEK(0));
                    ObjString* a = AS_STRING(PEEK(1));
                    
                    // Pop operands
                    stackTop -= 2; 

                    int length = a->length + b->length;

                    // FIX: Use reallocate (via explicit call since macro isn't here)
                    // need to cast to ensure the prototype matches
                    char* chars = (char*)reallocate(NULL, 0, length + 1);

                    memcpy(chars, a->chars, a->length);
                    memcpy(chars + a->length, b->chars, b->length);
                    chars[length] = '\0';

                    // Note: take_string allocates the object and adds it to VM list
                    // Since vm.objects relies on global vm struct, it's fine.
                    ObjString* result = take_string(chars, length);
                    *stackTop++ = OBJ_VAL(result);
                }

                else if (IS_INT(PEEK(0)) && IS_INT(PEEK(1))) {
                    // Integer Addition
                    int b = AS_INT(*(--stackTop));
                    int a = AS_INT(*(--stackTop));
                    *stackTop++ = INT_VAL(a + b);
                }

                else {
                    vm.ip = ip;
                    vm.stackTop = stackTop;
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            
            case OP_DIVIDE: {
                if (!IS_INT(stackTop[-1]) || !IS_INT(stackTop[-2])) {
                    vm.ip = ip; vm.stackTop = stackTop;
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int b = AS_INT(stackTop[-1]); // Peek
                if (b == 0) {
                    vm.ip = ip; vm.stackTop = stackTop;
                    runtimeError("Division by zero.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Now safe to pop and divide
                b = AS_INT(*(--stackTop));
                int a = AS_INT(*(--stackTop));
                *stackTop++ = INT_VAL(a / b);
                break;
            }

            case OP_NEGATE: {
                if (!IS_INT(stackTop[-1])) {
                    vm.ip = ip; vm.stackTop = stackTop;
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Unwrap, Negate, Wrap back
                stackTop[-1] = INT_VAL(-AS_INT(stackTop[-1]));
                break;
            }

            case OP_PRINT: {
                Value v = *(--stackTop);   // pop()
                printf("Out: ");    // prefix output for debugging
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
    reset_stack(); 
    return run();
}
