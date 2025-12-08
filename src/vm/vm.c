/**
 * @file vm.c
 * @brief Implementation of the Virtual Machine execution loop.
 *
 * This module contains stack manipulation utilities, the main interpreter loop,
 * and helper macros used for bytecode execution. 
 * 
 * The VM is a simple stack-based registerless interpreter 
 * similar to Lox/Wren-style VMs.
 * 
 * Design:
 *  - Values are on a single operand stack (vm.stack).
 *  - Each CallFrame describes one active function call.
 *  - frame->slots points into the stack where that frame's locals start.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vm/vm.h"
#include "vm/opcode.h"
#include "vm/common.h"
#include "vm/value.h"
#include "vm/object.h" // Need this to access ObjString struct for freeing
#include "vm/memory.h"
#include "vm/compiler.h"

// The global VM instance (single VM model)
VM vm;


/* ============================
 *  Stack & VM Initialization
 * ============================ */

/**
 * @brief Reset the VM operand stack to an empty state.
 */
static void reset_stack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

/**
 * @brief Initialize the VM 
 * 
 * (stack, GC state, object list).
 */
void init_vm(void) {
    reset_stack();
    vm.objects = NULL; // Initialize the tracker list

    // --- GC Init ---
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024; // Start GC at 1MB
}

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
    vm.grayStack = NULL;
}

// ====================
// Stack Operations
// ====================

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
        fprintf(stderr, "VM Error: Stack overflow.\n");
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


/* ============================
 *  Runtime Error & Calls
 * ============================ */


/**
 * @brief Prints out the error message to the terminal during function runtime
 * 
 * Unwind the VM and Includes a simple stack-trace by walking CallFrames.
 * 
 * @param format 
 */
static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Print Stack Trace
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;

        // IP points *past* current instruction, step back one
        size_t instruction = (size_t)(frame->ip - function->chunk.code - 1);
        int line = function->chunk.lines[instruction];

        fprintf(stderr, "[line %d] in ", line);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    reset_stack();
}

/**
 * @brief Call a function object with a given number of arguments.
 * 
 * Stack layout when this is called:
 *   [... previous values ...] [callee] [arg1] [arg2] ... [argN]
 * 
 * @param function ObjFunction to call
 * @param argCount number of arguments on top of the stack
 * @return true 
 * @return false 
 */
static bool call(ObjFunction* function, int argCount) {
    if (argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.", function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;

    // The slots start at where the arguments are on the stack
    frame->slots = vm.stackTop - argCount - 1; // -1 for the function itself (which is slot 0)
    return true;
}


/**
 * @brief Function to return a boolean based on the function call value
 * 
 * @param callee 
 * @param argCount 
 * @return true 
 * @return false 
 */
static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_FUNCTION: 
                return call(AS_FUNCTION(callee), argCount);

            // Add more cases in the future for Closures, Classes, Objects etc
            default:
                break; // Non-callable object
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}


// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief Helper macro: current frame.
 */
#define FRAME() (vm.frames[vm.frameCount - 1])

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
    // Read macros using local cached registers
    #define READ_BYTE() (*FRAME().ip++)

    // Read a constant from the current function's constant pool
    #define READ_CONSTANT() (FRAME().function->chunk.constants.values[READ_BYTE()])

    // Macro to Read 16-bit operand (Big Endian)
    #define READ_SHORT() \
        (FRAME().ip += 2, (uint16_t)((FRAME().ip[-2] << 8) | FRAME().ip[-1]))

    // PEEK macro to use local stackTop
    #define PEEK(i) peek(i)

    // Optimized binary operation macro
    // Note: using const Value ensures no aliasing surprises & better optimization
    #define BINARY_OP(op) \
        do { \
            if (!IS_INT(peek(0)) || !IS_INT(peek(1))) { \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            int b = AS_INT(pop()); \
            int a = AS_INT(pop()); \
            push(INT_VAL(a op b)); \
        } while (0)

    // Helper macro to assist with debugging stack operations and value pushing

    #define DEBUG_STACK() \
    do { \
        printf("STACK: "); \
        for (Value* s = vm.stack; s < vm.stackTop; s++) { \
            print_value(*s); \
            printf(" | "); \
        } \
        printf("\n"); \
    } while(0)

    for (;;) {
        // uncomment for viewing stack operations
        // DEBUG_STACK();
        // printf("OP: %d\n", *FRAME().ip);

        #ifdef DEBUG_TRACE_EXECUTION
                printf("          ");
                for (Value* slot = vm.stack; slot < stackTop; slot++) {
                    printf("[ ");
                    print_value(*slot);
                    printf(" ]");
                }
                printf("\n");

                printf("IP %04ld: Opcode %d\n", 
                        (long)(FRAME().ip - FRAME().function->chunk.code), 
                        *FRAME().*ip);
        #endif

        uint8_t instruction = READ_BYTE();

        switch (instruction) {
            /* --- Constants & literals --- */

            case OP_CONSTANT: {
                push(READ_CONSTANT());
                break;
            }

            // Conditional and equivalance logic;
            case OP_TRUE:  push(BOOL_VAL(true)); break;

            case OP_FALSE: push(BOOL_VAL(false)); break;


            /* --- Globals --- */

            // Global variable logic
            case OP_GET_GLOBAL: {
                uint8_t index = READ_BYTE();
                // Direct array access: O(1) speed
                push(vm.globals[index]);
                break;
            }

            case OP_SET_GLOBAL: {
                uint8_t index = READ_BYTE();
                // Assignment expressions evaluate to the assigned value.
                // We PEEK the value so it stays on the stack for usage.
                vm.globals[index] = PEEK(0);
                break;
            }


            /* -- Locals --- */
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE(); //Access the stack directly at the given index
                push(FRAME().slots[slot]); // relative to frame addressing
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                // Assignment = Expression hence we peek the stack
                FRAME().slots[slot] = PEEK(0);
                break;
            }


            /* --- Stack Cleanup --- */

            case OP_POP: {
                pop();
                break;
            }


            /* --- Control Flow --- */

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                FRAME().ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                // False is falsey. Everything else is truthy.
                // If false, jump. If true, fall through (and let next instruction execute).
                if (IS_BOOL(PEEK(0)) && !AS_BOOL(PEEK(0))) {
                    FRAME().ip += offset;
                }
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                FRAME().ip -= offset;
                break;
            }

            /* --- Calls --- */

            case OP_CALL: {
                uint8_t argCount = READ_BYTE();
                // The function object is at stackTop - argCount - 1
                if (!callValue(PEEK(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                // callValue() has pushed a new CallFrame with ip set to function->chunk.code
                // FRAME() now refers to the new frame; READ_* macros will use it automatically.
                break;
            }

            /* --- Comparisons & Logic --- */
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(values_equal(a, b)));
                break;
            }

            case OP_GREATER: {
                if (!IS_INT(PEEK(0)) || !IS_INT(PEEK(1))) { 
                    runtimeError("Operands must be numbers."); 
                    return INTERPRET_RUNTIME_ERROR; 
                }
                int b = AS_INT(pop());
                int a = AS_INT(pop());
                push(BOOL_VAL(a > b));
                break;
            }

            case OP_LESS: {
                if (!IS_INT(PEEK(0)) || !IS_INT(PEEK(1))) { 
                    runtimeError("Operands must be numbers."); 
                    return INTERPRET_RUNTIME_ERROR; 
                }
                int b = AS_INT(pop());
                int a = AS_INT(pop());
                push(BOOL_VAL(a < b));
                break;
            }

            case OP_NOT: {
                Value v = pop();
                if (!IS_BOOL(v)) {
                    runtimeError("Operand must be boolean.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(BOOL_VAL(!AS_BOOL(v)));
                break;
            }

            /* --- Arithmetic --- */

            case OP_ADD: {
                // Use PEEK macro instead of peek() function
                if (IS_STRING(PEEK(0)) && IS_STRING(PEEK(1))) {
                    // String Concatenation
                    ObjString* b = AS_STRING(pop());
                    ObjString* a = AS_STRING(pop());

                    // Concatenate the strings
                    ObjString* result = concatenate(a, b);
                    
                    push(OBJ_VAL(result));
                    break; // important not to fall through to int step afterwards
                }

                // Integer Addition
                else if (IS_INT(PEEK(0)) && IS_INT(PEEK(1))) {
                    BINARY_OP(+);
                    break;
                }

                else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            
            case OP_DIVIDE: {
                if (!IS_INT(PEEK(0)) || !IS_INT(PEEK(1))) {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int divisor = AS_INT(PEEK(0)); // Peek
                if (divisor == 0) {
                    runtimeError("Division by zero.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Now safe to pop and divide
                int b = AS_INT(pop());
                int a = AS_INT(pop());
                push(INT_VAL(a / b));
                break;
            }

            case OP_MODULO: {
                if (!IS_INT(PEEK(0)) || !IS_INT(PEEK(1))) {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int divisor = AS_INT(PEEK(0)); 
                if (divisor == 0) {
                    runtimeError("Modulo by zero.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int b = AS_INT(pop());
                int a = AS_INT(pop());
                push(INT_VAL(a % b));
                break;
            }

            case OP_NEGATE: {
                if (!IS_INT(PEEK(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Unwrap, Negate, Wrap back
                int value = AS_INT(pop());
                push(INT_VAL(-value));
                break;
            }

            case OP_PRINT: {
                // Value v = *(--stackTop);   // pop()
                // printf("Out: ");    // prefix output for debugging
                print_value(pop());
                printf("\n");
                break;
            }

            case OP_RETURN: {
                // 1. Pop the function's return value (top of stack inside the function)
                Value result = pop();

                // 2. Grab the current frame BEFORE popping it
                CallFrame* frame = &FRAME();

                // 3. Pop this call frame
                vm.frameCount--;

                // 4. If we just returned from the top-level script, we're done
                if (vm.frameCount == 0) {
                    // vm.stackTop = vm.stack;
                    // push the result back for inspection
                    push(result);
                    return INTERPRET_OK;
                }

                // 5. Discard callee + args + locals:
                //    shrink stack back to the callee slot of this frame
                vm.stackTop = frame->slots;

                // 6. Push the return value in their place (overwriting callee slot)
                push(result);

                // 7. Continue running caller frame (FRAME() now refers to the caller)
                break;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef BINARY_OP
    #undef FRAME
    #undef PEEK
    #undef DEBUG_STACK
}

/**
 * @brief Load a bytecode chunk into the VM and begin execution.
 *
 * @param source pointer to the entire source to compile it to a Function, and runs it.
 * @return InterpretResult OK or runtime error state.
 */
InterpretResult interpret(ObjFunction* function) {
    reset_stack(); 

    //  Compiler returns an ObjFunction
    // ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    // push(OBJ_VAL(function));

    vm.frameCount = 0;
    
    CallFrame* frame = &vm.frames[vm.frameCount++];

    frame->function = function;   // compiled function containing chunk
    frame->ip = function->chunk.code;
    frame->slots = vm.stack;      // slots start at base of stack

    return run();
}
