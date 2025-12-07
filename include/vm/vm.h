/**
 * @file vm.h
 * @brief Defines the Virtual Machine state and execution interface.
 * 
 * The VM will run in an infinite loop of Fetch -> Decode -> Execute
 * 
 * The VM runs a simple stack-based bytecode:
 *   - Values live on a single operand stack.
 *   - Each function call has a CallFrame pointing into that stack.
 *   - Bytecode lives inside ObjFunction->chunk.
 */

#ifndef VM_VM_H
#define VM_VM_H

#include "chunk.h"
#include "value.h" // Value needs to be fully defined
#include "object.h" // VM needs to know about Objects

#define STACK_MAX 256       // Maximum number of global variables allowed in a script
#define GLOBALS_MAX 256     // Max recursion depth
#define FRAMES_MAX 64       // Max call depth (number of CallFrames)

/**
 * @brief It represents a single function call in the stack
 * @struct CallFrame 
 */
typedef struct {
    ObjFunction* function; // The function being run
    uint8_t* ip;           // Instruction pointer *inside* that function's chunk
    Value* slots;          // Pointer to the start of this frame's local variables on the stack
} CallFrame;

/**
 * @struct VM
 * @brief The Virtual Machine state.
 */
typedef struct {
    CallFrame frames[FRAMES_MAX];   // Call Stack
    int frameCount;                 // number of frames
    
    Value stack[STACK_MAX];         // The operand stack
    Value* stackTop;                // Points to the top of the stack
    Value globals[GLOBALS_MAX];     // Compiler resolves names ("x") to indices (0) and stores it here
    Obj* objects;                   // Object Tracking: stores the Head of the linked list of all allocated objects

    // GC States
    int grayCount;              // Number of objects in the worklist
    int grayCapacity;           // Capacity of the worklist
    Obj** grayStack;            // The worklist (stack of gray objects)
    
    size_t bytesAllocated;      // Total bytes currently allocated
    size_t nextGC;              // Threshold to trigger the next collection
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

// VM Lifecycle
void init_vm(void);
void free_vm(void);

// --- Stack Operations ---
void push(Value value);
Value pop(void);
Value peek(int distance); // Look at stack without popping

/**
 * @brief Executes a chunk of bytecode.
 */
InterpretResult interpret(ObjFunction* function); 

#endif // VM_VM_H