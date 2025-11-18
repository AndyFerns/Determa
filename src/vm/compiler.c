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

#include "vm/compiler.h"
#include "vm/chunk.h"
#include "vm/opcode.h"
#include "ast.h"
#include "token.h"
#include <stdio.h>

// State for the compiler
typedef struct {
    Chunk* chunk;
    int hadError;
} Compiler;

// --- Helper Functions ---

/**
 * @brief Emits a single byte (OpCode) to the chunk.
 */
static void emit_byte(Compiler* compiler, uint8_t byte, int line) {
    write_chunk(compiler->chunk, byte, line);
}

/**
 * @brief Emits an OpCode followed by a constant index.
 */
static void emit_constant(Compiler* compiler, Value value, int line) {
    // 1. Add the actual value to the constant pool
    int constantIndex = add_constant(compiler->chunk, value);
    
    // 2. Emit the instruction and the operand (index)
    emit_byte(compiler, OP_CONSTANT, line);
    // Note: We currently assume the constant index fits in a single byte (max 256 constants)
    emit_byte(compiler, (uint8_t)constantIndex, line); 
}

/**
 * @brief Emits an arithmetic opcode based on the token type.
 */
static void emit_binary_op(Compiler* compiler, TokenType opType, int line) {
    switch (opType) {
        case TOKEN_PLUS:  emit_byte(compiler, OP_ADD, line); break;
        case TOKEN_MINUS: emit_byte(compiler, OP_SUBTRACT, line); break;
        case TOKEN_STAR:  emit_byte(compiler, OP_MULTIPLY, line); break;
        case TOKEN_SLASH: emit_byte(compiler, OP_DIVIDE, line); break;
        default: break; // Should not happen after type checking
    }
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
            emit_constant(compiler, n->value, n->node.line);
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
        
        // TODO: Handle VAR_ACCESS later
        
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
        case NODE_VAR_DECL: {
            // Var decls require no runtime code for now (handled by VM stack later)
            // But we must compile the initializer expression
            AstNodeVarDecl* n = (AstNodeVarDecl*)stmt;
            compile_expression(compiler, n->init);
            // TODO: Emit OP_DEFINE_GLOBAL/LOCAL instruction here
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
            // Pop the resulting value of the expression, since it's a statement
            // TODO: Emit OP_POP
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
    }
    // Every chunk must end with a return instruction
    emit_byte(compiler, OP_RETURN, 0); 
}

