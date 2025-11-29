/**
 * @file vm.h
 * @brief Defines the Virtual Machine state and execution interface.
 * 
 * The VM will run in an infinite loop of Fetch -> Decode -> Execute
 */

#ifndef VM_VM_H
#define VM_VM_H

#include "chunk.h"
#include "value.h" // Value needs to be fully defined
#include "object.h" // VM needs to know about Objects

#define STACK_MAX 256
// Maximum number of global variables allowed in a script
#define GLOBALS_MAX 256 

/**
 * @struct VM
 * @brief The Virtual Machine state.
 */
typedef struct {
    Chunk* chunk;               // The chunk of bytecode being executed
    uint8_t* ip;                // Instruction Pointer (points to next byte to read)
    Value stack[STACK_MAX];     // The operand stack
    Value* stackTop;            // Points to the top of the stack
    Value globals[GLOBALS_MAX]; // Compiler resolves names ("x") to indices (0) and stores it here
    Obj* objects;               // Object Tracking: stores the Head of the linked list of all allocated objects
} VM;

/**
 * @enum InterpretResult
 * @brief Result codes for the VM execution.
 */
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// Expose the global VM instance for testing purposes (inspecting the stack)
extern VM vm;

void init_vm();
void free_vm();

// --- Stack Operations ---
void push(Value value);
Value pop();
Value peek(int distance); // Look at stack without popping

/**
 * @brief Executes a chunk of bytecode.
 */
InterpretResult interpret(Chunk* chunk);

#endif // VM_VM_H