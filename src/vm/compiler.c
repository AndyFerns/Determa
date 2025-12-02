/**
 * @file compiler.c
 * @author your name (you@domain.com)
 * @brief A compiler stub used to Walk AST and emit bytecode
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>   // for uint8_t

#include "vm/compiler.h"
#include "vm/chunk.h"
#include "vm/opcode.h"
#include "vm/object.h" // Need object API
#include "vm/memory.h" // Needed for mark_value
#include "ast.h"
#include "token.h"


// This exists ONLY during compilation to map "x" -> 0.
typedef struct {
    char* name;
    int length;
    int index;
} CompilerSymbol;

// Simple linear symbol table for globals
static CompilerSymbol globalSymbols[256];
static int globalCount;

// State for the compiler
typedef struct {
    Chunk* chunk;
    int hadError;
} Compiler;




// --- Active Compiler Tracking for GC ---
// required so that the gc can find the constants inside the chunk currently being compiled
static Compiler* current = NULL;


/**
 * @brief Function to mark all constants in the chunk currently being compiled
 * Prevents the GC from deleting strings just created but not compiled yet
 * 
 * @return void
 */
void mark_compiler_roots() {
    if (current == NULL) return;
    
    Chunk* chunk = current->chunk;
    for (int i = 0; i < chunk->constants.count; i++) {
        mark_value(chunk->constants.values[i]);
    }
}




// ========================
// --- Helper Functions ---
// ========================

/**
 * @brief Function to assign global count as 0. Used while initializing the compiler
 * 
 * @return void
 */
void init_compiler(void) {
    // globalCount = 0;
}


/**
 * @brief Emits a single byte (OpCode) to the chunk.
 */
static void emit_byte(Compiler* compiler, uint8_t byte, int line) {
    write_chunk(compiler->chunk, byte, line);
}

/**
 * @brief Function to Emit two bytes (used for jump offsets)
 */
static void emit_bytes(Compiler* compiler, uint8_t byte1, uint8_t byte2, int line) {
    emit_byte(compiler, byte1, line);
    emit_byte(compiler, byte2, line);
}

/**
 * @brief Emits an OpCode followed by a constant index.
 */
static void emit_constant(Compiler* compiler, Value value, int line) {
    // 1. Add the actual value to the constant pool
    int constantIndex = add_constant(compiler->chunk, value);

    if (constantIndex < 0) {
        fprintf(stderr, "Compiler Error: failed to add constant (index < 0)\n");
        compiler->hadError = 1;
        return;
    }

    if (constantIndex > 0xFF) {
        fprintf(stderr, "Compiler Error: too many constants (>=256). Add wide-constant support.\n");
        compiler->hadError = 1;
        return;
    }
    
    // 2. Emit the instruction and the operand (index)
    emit_byte(compiler, OP_CONSTANT, line);
    // Note: We currently assume the constant index fits in a single byte (max 256 constants)
    emit_byte(compiler, (uint8_t)constantIndex, line); 
}


/**
 * @brief Emit a jump instruction with a placeholder operand. 
 * 
 * @param compiler 
 * @param instruction 
 * @param line 
 * @return int The index of the placeholder so we can patch it later.
 */
static int emit_jump(Compiler* compiler, uint8_t instruction, int line) {
    emit_byte(compiler, instruction, line);
    emit_bytes(compiler, 0xff, 0xff, line); // helper to emit 2 placeholder bytes
    return compiler->chunk->count - 2;
}


/**
 * @brief Function to Go back to 'offset' and write the jump distance to current location
 * 
 * @param compiler 
 * @param offset 
 */
static void patch_jump(Compiler* compiler, int offset) {
    // -2 to adjust for the jump offset itself
    int jump = compiler->chunk->count - offset - 2;

    if (jump > UINT16_MAX) {
        fprintf(stderr, "Too much code to jump over.\n");
        compiler->hadError = 1;
    }

    compiler->chunk->code[offset] = (jump >> 8) & 0xff;
    compiler->chunk->code[offset + 1] = jump & 0xff;
}

/**
 * @brief Emit a backward jump loop (used in While conditional blocks)
 * 
 * @param compiler 
 * @param loopStart 
 * @param line 
 */
static void emit_loop(Compiler* compiler, int loopStart, int line) {
    emit_byte(compiler, OP_LOOP, line);

    int offset = compiler->chunk->count - loopStart + 2;
    if (offset > UINT16_MAX) {
        fprintf(stderr, "Loop body too large.\n");
        compiler->hadError = 1;
    }

    emit_bytes(compiler, (offset >> 8) & 0xff, offset & 0xff, line); // helper to clear two offsets
}

/**
 * @brief Emits an arithmetic opcode based on the token type.
 */
static void emit_binary_op(Compiler* compiler, TokenType opType, int line) {
    switch (opType) {
        // --- Arithmetic Operations ---
        case TOKEN_PLUS:  emit_byte(compiler, OP_ADD, line); break;
        case TOKEN_MINUS: emit_byte(compiler, OP_SUBTRACT, line); break;
        case TOKEN_STAR:  emit_byte(compiler, OP_MULTIPLY, line); break;
        case TOKEN_SLASH: emit_byte(compiler, OP_DIVIDE, line); break;

        // --- Comparisons ---
        case TOKEN_EQUAL_EQUAL:   emit_byte(compiler, OP_EQUAL, line); break;
        case TOKEN_GREATER:       emit_byte(compiler, OP_GREATER, line); break;
        case TOKEN_LESS:          emit_byte(compiler, OP_LESS, line); break;

        // Composite Ops (Syntactic Sugar in Bytecode)
        case TOKEN_BANG_EQUAL:    
            emit_byte(compiler, OP_EQUAL, line); 
            emit_byte(compiler, OP_NOT, line);   
            break;
        case TOKEN_GREATER_EQUAL: 
            emit_byte(compiler, OP_LESS, line);    // !(a < b)
            emit_byte(compiler, OP_NOT, line);
            break;
        case TOKEN_LESS_EQUAL:    
            emit_byte(compiler, OP_GREATER, line); // !(a > b)
            emit_byte(compiler, OP_NOT, line);
            break;

        default:
            fprintf(stderr, "Compiler Error: Unsupported binary operator token %d\n", opType);
            compiler->hadError = 1;
            break;
    }
}

static int resolve_global(Compiler* compiler, Token name) {
    (void)compiler;
    for (int i = 0; i < globalCount; i++) {
        CompilerSymbol* existing = &globalSymbols[i];
        // String comparison
        if (existing->length == name.length && 
            strncmp(existing->name, name.lexeme, name.length) == 0) {
            return globalSymbols[i].index;
        }
    }
    return -1;
}

// Creates a new global variable mapping
static int define_global(Compiler* compiler, Token name) {
    if (globalCount >= 256) {
        fprintf(stderr, "Too many global variables.\n");
        compiler->hadError = 1;
        return 0;
    }
    // Check if already defined (optional, but good for safety)
    int existing = resolve_global(compiler, name);
    if (existing != -1) return existing;

    int index = globalCount;
    
    // ALLOCATE MEMORY for the name
    char* nameCopy = (char*)malloc(name.length + 1);
    if (!nameCopy) {
        fprintf(stderr, "Out of memory while defining global variable.\n");
        compiler->hadError = 1;
        return -1;
    }

    memcpy(nameCopy, name.lexeme, name.length);
    nameCopy[name.length] = '\0';

    globalSymbols[index].name = nameCopy;
    globalSymbols[index].length = name.length;
    globalSymbols[index].index = index;
    
    globalCount++;
    return index;
}


// --- Recursive Compilation Logic (The AST Walker) ---

/**
 * @brief Compiles an expression node recursively.
 */
static void compile_expression(Compiler* compiler, AstNode* expr) {
    if (compiler->hadError || expr == NULL) return;

    switch (expr->type) {
        case NODE_INT_LITERAL: {
            AstNodeIntLiteral* n = (AstNodeIntLiteral*)expr;
            // Emit instruction to load the constant onto the stack
            emit_constant(compiler, INT_VAL(n->value), (n->node.line));
            break;
        }

        // Compile string
        case NODE_STRING_LITERAL: {
            AstNodeStringLiteral* n = (AstNodeStringLiteral*)expr;
            // Create a String Object on the heap
            ObjString* stringObj = copy_string(n->value, (int)strlen(n->value));
            // Wrap it in a Value and emit as a constant
            emit_constant(compiler, OBJ_VAL(stringObj), n->node.line);
            break;
        }

        case NODE_BOOL_LITERAL: {
            AstNodeBoolLiteral* n = (AstNodeBoolLiteral*)expr;
            emit_byte(compiler, n->value ? OP_TRUE : OP_FALSE, n->node.line);
            break;
        }

        // Handles variable access (x + 1)
        case NODE_VAR_ACCESS: {
            AstNodeVarAccess* n = (AstNodeVarAccess*)expr;
            
            // 1. Look up the index
            int index = resolve_global(compiler, n->name);
            if (index == -1) {
                // This should have been caught by TypeChecker, but safety first
                fprintf(stderr, "Compiler Error: Undefined variable '%.*s'\n", n->name.length, n->name.lexeme);
                compiler->hadError = 1;
                return;
            }
            
            // 2. Emit OP_GET_GLOBAL [INDEX]
            emit_byte(compiler, OP_GET_GLOBAL, n->node.line);
            emit_byte(compiler, (uint8_t)index, n->node.line);
            break;
        }

        case NODE_UNARY_OP: {
            AstNodeUnaryOp* n = (AstNodeUnaryOp*)expr;
            compile_expression(compiler, n->operand);
            if (n->op.type == TOKEN_MINUS) {
                // '-' negate operator
                emit_byte(compiler, OP_NEGATE, n->op.line);
            } else if (n->op.type == TOKEN_BANG) {
                // ! not operator
                emit_byte(compiler, OP_NOT, n->op.line);
            }
            break;
        }

        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)expr;
            
            // 1. Compile the left operand (Pushes its result to stack)
            compile_expression(compiler, n->left);
            
            // 2. Compile the right operand (Pushes its result to stack)
            compile_expression(compiler, n->right);
            
            // 3. Emit the arithmetic operation
            emit_binary_op(compiler, n->op.type, n->op.line);
            break;
        }
        
        default:
            fprintf(stderr, "Compiler Error: Unhandled expression node type %d\n", expr->type);
            compiler->hadError = 1;
            break;
    }
}

/**
 * @brief Compiles a statement node.
 */
static void compile_statement(Compiler* compiler, AstNode* stmt) {
    if (compiler->hadError || stmt == NULL) return;

    switch (stmt->type) {
        // --- Block ---
        case NODE_BLOCK: {
            AstNodeBlock* block = (AstNodeBlock*)stmt;
            for (int i = 0; i < block->statement_count; i++) {
                compile_statement(compiler, block->statements[i]);
            }
            break;
        }

        // --- If Statement ---
        case NODE_IF: {
            AstNodeIf* n = (AstNodeIf*)stmt;
            // 1. Compile Condition
            compile_expression(compiler, n->condition);
            
            // 2. Jump over "then" if false
            int thenJump = emit_jump(compiler, OP_JUMP_IF_FALSE, n->node.line);
            emit_byte(compiler, OP_POP, n->node.line); // Clean up condition if true
            
            // 3. Compile "then" block
            compile_statement(compiler, n->thenBranch);
            
            // 4. Jump over "else"
            int elseJump = emit_jump(compiler, OP_JUMP, n->node.line);
            
            // 5. Patch start of "else" (target of thenJump)
            patch_jump(compiler, thenJump);
            emit_byte(compiler, OP_POP, n->node.line); // Clean up condition if false (jump landed here)
            
            // 6. Compile "else" block (if exists)
            if (n->elseBranch) {
                compile_statement(compiler, n->elseBranch);
            }
            
            // 7. Patch end of if (target of elseJump)
            patch_jump(compiler, elseJump);
            break;
        }

        // --- While Loop ---
        case NODE_WHILE: {
            AstNodeWhile* n = (AstNodeWhile*)stmt;
            int loopStart = compiler->chunk->count; // Mark start of loop
            
            // 1. Condition
            compile_expression(compiler, n->condition);
            
            // 2. Jump out if false
            int exitJump = emit_jump(compiler, OP_JUMP_IF_FALSE, n->node.line);
            emit_byte(compiler, OP_POP, n->node.line); // Clean up condition if true
            
            // 3. Body
            compile_statement(compiler, n->body);
            
            // 4. Loop back
            emit_loop(compiler, loopStart, n->node.line);
            
            // 5. Patch exit
            patch_jump(compiler, exitJump);
            emit_byte(compiler, OP_POP, n->node.line); // Clean up condition if false
            break;
        }

        case NODE_VAR_DECL: {
            AstNodeVarDecl* n = (AstNodeVarDecl*)stmt;
            
            // 1. Compile the initializer (pushes 10 to stack)
            compile_expression(compiler, n->init);
            
            // 2. Assign a new index to 'x'
            int index = define_global(compiler, n->name);
            if (index == -1) {
                // define_global already set hadError
                return;
            }
            
            // 3. Emit OP_SET_GLOBAL [INDEX] to pop 10 and store it in slot 0
            emit_byte(compiler, OP_SET_GLOBAL, n->node.line);
            emit_byte(compiler, (uint8_t)index, n->node.line);
            break;
        }
        
        case NODE_PRINT_STMT: {
            AstNodePrintStmt* n = (AstNodePrintStmt*)stmt;
            // 1. Compile the expression (Pushes value to stack)
            compile_expression(compiler, n->expression);
            // 2. Emit the print instruction
            emit_byte(compiler, OP_PRINT, n->node.line);
            break;
        }

        case NODE_EXPR_STMT: {
            AstNodeExprStmt* n = (AstNodeExprStmt*)stmt;
            compile_expression(compiler, n->expression);
            // Expression as statement should pop its value
            emit_byte(compiler, OP_POP, n->node.line); 
            break;
        }

        default:
            fprintf(stderr, "Compiler Error: Unhandled statement node type %d\n", stmt->type);
            compiler->hadError = 1;
            break;
    }
}

/**
 * @brief Compiles the root PROGRAM node.
 */
static void compile_program(Compiler* compiler, AstNode* root) {
    AstNodeProgram* prog = (AstNodeProgram*)root;
    for (int i = 0; i < prog->statement_count; i++) {
        compile_statement(compiler, prog->statements[i]);
        if (compiler->hadError) return;
    }
    // Every chunk must end with a return instruction
    emit_byte(compiler, OP_RETURN, 0); 
}


/**
 * @brief Public interface to the compiler.
 */
int compile_ast(struct AstNode* ast, Chunk* chunk) {
    Compiler compiler;
    compiler.chunk = chunk;
    compiler.hadError = 0;
    // no need to reset global Count anymore as it persists
    // compiler.globalCount = 0;   // start fresh for every compile

    // set active compiler for gc
    current = &compiler; 

    if (ast->type != NODE_PROGRAM) {
        fprintf(stderr, "Compiler Error: AST root must be PROGRAM\n");
        return 0;
    }

    compile_program(&compiler, ast);

    // clear active compiler after completed compilation
    current = NULL;
    
    if (compiler.hadError) {
        // If an error occurred, the chunk is incomplete/corrupt
        free_chunk(chunk);
        return 0;
    }
    return 1;
}


/**
 * @brief Free Global symbols helper function
 * call on shutdown or before resetting the compiler
 */
void free_global_symbols(void) {
    for (int i = 0; i < globalCount; i++) {
        free(globalSymbols[i].name);
        globalSymbols[i].name = NULL;
        globalSymbols[i].length = 0;
        globalSymbols[i].index = -1;
    }
    globalCount = 0;
}
