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

/**
 * @brief Reset the VM operand stack to an empty state.
 */
static void reset_stack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

/**
 * @brief Initialize the VM (reset stack, clear state).
 */
void init_vm() {
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
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    reset_stack();
}

/**
 * @brief method to handle function calls inside blocks
 * 
 * @param function 
 * @param argCount 
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
    // Switching from chunk-based interpretation to Frame (function) based
    #define FRAME() (vm.frames[vm.frameCount - 1]) 

    // Read macros using local cached registers
    #define READ_BYTE() (*FRAME().ip++)
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
                push(READ_CONSTANT());
                break;
            }

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

            // Conditional and equivalance logic;
            case OP_TRUE:  push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;

            // --- Stack Cleanup ---
            case OP_POP: {
                pop();
                break;
            }

            // --- Control Flow ---
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

            // --- Conditional Ops ---
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

            case OP_ADD: {
                // Use PEEK macro instead of peek() function
                if (IS_STRING(PEEK(0)) && IS_STRING(PEEK(1))) {
                    // String Concatenation
                    ObjString* b = AS_STRING(pop());
                    ObjString* a = AS_STRING(pop());
                    
                    push(OBJ_VAL(concatenate(a, b)));
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
                int b = AS_INT(PEEK(0)); // Peek
                if (b == 0) {
                    runtimeError("Division by zero.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Now safe to pop and divide
                BINARY_OP(/);
                break;
            }

            case OP_MODULO: {
                if (!IS_INT(PEEK(0)) || !IS_INT(PEEK(1))) {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int b = AS_INT(PEEK(0)); 
                if (b == 0) {
                    runtimeError("Modulo by zero.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                BINARY_OP(%);
                break;
            }

            case OP_NEGATE: {
                if (!IS_INT(PEEK(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Unwrap, Negate, Wrap back
                push(INT_VAL(-AS_INT(pop())));
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
                Value result = pop();
                vm.frameCount--;

                if (vm.frameCount == 0) {
                    pop(); // Pop the script function
                    return INTERPRET_OK;
                }

                // Discard the CallFrame's locals from the stack
                vm.stackTop = FRAME().slots;
                push(result); // Push the return value
                break;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
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
