/**
 * @file ast.c
 * @author Andrew Fernandes
 * @brief Defines the Data structures for the Abstract Syntax Tree (AST)
 * 
 * This file includes all the function definitions and helper functions for AST parsing
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Private Helper to print with indentation ---
static void print_ast_recursive(AstNode* node, int indent) {
    if (node == NULL) {
        printf("%*sNULL_NODE\n", indent, "");
        return;
    }

    // Indent
    printf("%*s", indent, "");

    switch (node->type) {
        case NODE_PROGRAM: {
            AstNodeProgram* p = (AstNodeProgram*)node;
            printf("PROGRAM (%d statements)\n", p->statement_count);
            for (int i = 0; i < p->statement_count; i++) {
                print_ast_recursive(p->statements[i], indent + 2);
            }
            break;
        }

        case NODE_BLOCK: {
            AstNodeBlock* b = (AstNodeBlock*)node;
            printf("BLOCK (%d statements)\n", b->statement_count);
            for (int i = 0; i < b->statement_count; i++) {
                print_ast_recursive(b->statements[i], indent + 2);
            }
            break;
        }

        case NODE_INT_LITERAL: {
            AstNodeIntLiteral* n = (AstNodeIntLiteral*)node;
            printf("INT_LITERAL: %d\n", n->value);
            break;
        }

        case NODE_STRING_LITERAL: {
            AstNodeStringLiteral* n = (AstNodeStringLiteral*)node;
            printf("STRING_LITERAL: \"%s\"\n", n->value);
            break;
        }

        case NODE_BOOL_LITERAL: {
            AstNodeBoolLiteral* n = (AstNodeBoolLiteral*)node;
            printf("BOOL_LITERAL: %s\n", n->value ? "true" : "false");
            break;
        }

        case NODE_UNARY_OP: {
            AstNodeUnaryOp* n = (AstNodeUnaryOp*)node;
            printf("UNARY_OP: %s\n", token_type_to_string(n->op.type));
            print_ast_recursive(n->operand, indent + 2);
            break;
        }

        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)node;
            printf("BINARY_OP: %s\n", token_type_to_string(n->op.type));
            print_ast_recursive(n->left, indent + 2);
            print_ast_recursive(n->right, indent + 2);
            break;
        }

        case NODE_VAR_DECL: {
            AstNodeVarDecl* n = (AstNodeVarDecl*)node;
            printf("VAR_DECL: %.*s\n", n->name.length, n->name.lexeme);
            print_ast_recursive(n->init, indent + 2);
            break;
        }

        case NODE_VAR_ACCESS: {
            AstNodeVarAccess* n = (AstNodeVarAccess*)node;
            printf("VAR_ACCESS: %.*s\n", n->name.length, n->name.lexeme);
            break;
        }

        case NODE_VAR_ASSIGN: {
            AstNodeVarAssign* n = (AstNodeVarAssign*)node;
            printf("VAR_ASSIGN: %.*s\n", n->name.length, n->name.lexeme);
            print_ast_recursive(n->expression, indent + 2);
            break;
        }

        case NODE_PRINT_STMT: {
            AstNodePrintStmt* n = (AstNodePrintStmt*)node;
            printf("PRINT_STMT\n");
            print_ast_recursive(n->expression, indent + 2);
            break;
        }
        
        case NODE_EXPR_STMT: { 
            AstNodeExprStmt* n = (AstNodeExprStmt*)node;
            printf("EXPR_STMT\n");
            print_ast_recursive(n->expression, indent + 2);
            break;
        }

        case NODE_IF: {
            AstNodeIf* n = (AstNodeIf*)node;
            printf("IF\n");
            print_ast_recursive(n->condition, indent + 2);
            printf("%*sTHEN:\n", indent + 2, "");
            print_ast_recursive(n->thenBranch, indent + 4);
            if (n->elseBranch) {
                printf("%*sELSE:\n", indent + 2, "");
                print_ast_recursive(n->elseBranch, indent + 4);
            }
            break;
        }

        case NODE_WHILE: {
            AstNodeWhile* n = (AstNodeWhile*)node;
            printf("WHILE\n");
            print_ast_recursive(n->condition, indent + 2);
            printf("%*sDO:\n", indent + 2, "");
            print_ast_recursive(n->body, indent + 4);
            break;
        }

        // -- Function Creation Suite ---
        case NODE_FUNC_DECL: {
            AstNodeFuncDecl* n = (AstNodeFuncDecl*)node;
            printf("FUNC_DECL: %.*s (", n->name.length, n->name.lexeme);
            // Print params
            for(int i=0; i<n->param_count; i++) {
                printf("%.*s%s", n->params[i].length, n->params[i].lexeme, (i<n->param_count-1)?", ":"");
            }
            printf(") : %s\n", type_to_string(n->returnType));
            print_ast_recursive(n->body, indent + 2);
            break;
        }

        case NODE_RETURN: {
            AstNodeReturn* n = (AstNodeReturn*)node;
            printf("RETURN\n");
            print_ast_recursive(n->value, indent + 2);
            break;
        }
        
        case NODE_CALL: {
            AstNodeCall* n = (AstNodeCall*)node;
            printf("CALL: %.*s\n", n->callee.length, n->callee.lexeme);
            for(int i=0; i<n->arg_count; i++) {
                print_ast_recursive(n->args[i], indent + 2);
            }
            break;
        }
        
        // add more cases as the AST node implementation grows
        default:
            printf("UNKNOWN_NODE\n");
            break;
    }
}

/**
 * @brief Prints an AST tree to the console (for debugging)
 */
void print_ast(AstNode* node) {
    print_ast_recursive(node, 0);
}

/**
 * @brief Frees an entire AST tree recursively
 * 
 * This performs a full post-order traversal and frees:
 *  - All child nodes
 *  - Dynamic arrays (in NODE_PROGRAM)
 *  - The node itself
 * 
 * @param AstNode* Pointer to the abstract syntax tree root node
 */
void free_ast(AstNode* node) {
    if (node == NULL) return;

    // Use post-order traversal to free
    switch (node->type) {
        case NODE_PROGRAM: {
            // Program holds a list of statements
            AstNodeProgram* prog = (AstNodeProgram*)node;

            // Free each statement
            for (int i = 0; i < prog->statement_count; i++) {
                free_ast(prog->statements[i]);
            }

            // Free the dynamic statement array
            free(prog->statements);
            break;
        }


        /* ================================
         *  BLOCK OF STATEMENTS
         *  recursively walk down the ast and free each subsequent statement
         * ================================ */
        case NODE_BLOCK: {
            AstNodeBlock* b = (AstNodeBlock*)node;
            for (int i = 0; i < b->statement_count; i++) free_ast(b->statements[i]);
            free(b->statements);
            break;
        }


        /* ================================
         *  INTEGER LITERAL
         *  (No children)
         * ================================ */
        // No children, just free the node
        case NODE_INT_LITERAL:
            break;


        /* ================================
         *  STRING LITERAL
         *  (No children)
         *  Free the string copy only
         * ================================ */
        case NODE_STRING_LITERAL: {
            free(((AstNodeStringLiteral*)node)->value); // Free the string copy
            break;
        }


        /* ================================
         *  BOOL LITERAL
         *  (No children)
         *  // Nothing to free inside
         * ================================ */
        case NODE_BOOL_LITERAL: {
            break;
        }
        

        /* ================================
         *  BINARY OPERATION
         *  e.g., left + right
         * ================================ */
        // Standard binary operation left 'op' right
        case NODE_BINARY_OP: {
            AstNodeBinaryOp* n = (AstNodeBinaryOp*)node;
            free_ast(n->left);
            free_ast(n->right);
            break;
        }

        /* ================================
         *  VARIABLE DECLARATION
         *  var x = init;
         * ================================ */
        case NODE_VAR_DECL: {
            AstNodeVarDecl* decl = (AstNodeVarDecl*)node;
            free_ast(decl->init);  // May be NULL
            break;
        }

        /* ================================
         *  VARIABLE ASSIGNMENT
         *  x = expr;
         * ================================ */
        case NODE_VAR_ASSIGN: {
            AstNodeVarAssign* assign = (AstNodeVarAssign*)node;
            free_ast(assign->expression);
            break;
        }

        /* ================================
         *  VARIABLE ACCESS
         *  (No children)
         * ================================ */
        case NODE_VAR_ACCESS: {
            // Token holds no heap memory → no action needed
            break;
        }

        /* ================================
         *  PRINT STATEMENT
         *  print expr;
         * ================================ */
        case NODE_PRINT_STMT: {
            AstNodePrintStmt* n = (AstNodePrintStmt*)node;
            free_ast(n->expression);
            break;
        }

        /* ================================
         *  EXPRESSION STATEMENT
         *  expr;
         * ================================ */
        case NODE_EXPR_STMT: {
            AstNodeExprStmt* est = (AstNodeExprStmt*)node;
            free_ast(est->expression);
            break;
        }

        /* ================================
         *  IF CONDITIONAL BLOCK
         *  recursively free the condition, 
         *  'then' branch and 'else' branch
         * ================================ */
        case NODE_IF: {
            AstNodeIf* n = (AstNodeIf*)node;
            free_ast(n->condition);
            free_ast(n->thenBranch);
            free_ast(n->elseBranch);
            break;
        }

        /* ================================
         *  WHILE CONDITIONAL BLOCK
         *  recursively free the condition and the body statements
         * ================================ */
        case NODE_WHILE: {
            AstNodeWhile* n = (AstNodeWhile*)node;
            free_ast(n->condition);
            free_ast(n->body);
            break;
        }

        case NODE_FUNC_DECL: {
            AstNodeFuncDecl* fn = (AstNodeFuncDecl*)node;
            free(fn->params);
            free_ast(fn->body);
            break;
        }

        // We will add more cases here
        default:
            // No-op
            break;
    }
    
    // Free the node itself
    free(node);
}


// =============================
// --- Constructor functions ---
// =============================


/**
 * @brief Stores a "Program" as a list of expressions
 * 
 * @param statements 
 * @param count 
 * @return AstNode* 
 */
AstNode* new_program_node(AstNode** statements, int count) {
    AstNodeProgram* node = (AstNodeProgram*)malloc(sizeof(AstNodeProgram));
    if (!node) return NULL;

    node->node.type = NODE_PROGRAM;
    // node->node.line = line;

    // Initial dynamic array capacity
    node->capacity = (count > 0) ? count * 2 : 8;
    node->statements = (AstNode**)malloc(sizeof(AstNode*) * node->capacity);

    if (!node->statements) {
        free(node);
        return NULL;
    }

    // Copy initial statements if provided
    if (statements && count > 0) {
        memcpy(node->statements, statements, sizeof(AstNode*) * count);
        node->statement_count = count;
    } else {
        node->statement_count = 0;
    }

    return (AstNode*)node;
}

/**
 * @brief Adds a statement to the Program node
 * 
 * Helper Function for NODE_PROGRAM
 * 
 * @param program_node pointer to an AstNodeProgram (base AstNode ptr)
 * @param statement Pointer to a list of statements required to be appended
 */
void program_add_statement(AstNode* program_node, AstNode* statement) {
    if (!program_node || program_node->type != NODE_PROGRAM) return;

    AstNodeProgram* p = (AstNodeProgram*)program_node;

    // Expand array if necessary
    if (p->statement_count >= p->capacity) {
        p->capacity *= 2;
        AstNode** new_block = (AstNode**)realloc(
            p->statements,
            sizeof(AstNode*) * p->capacity
        );

        if (!new_block) {
            // realloc failed → KEEP OLD BLOCK, DO NOT MODIFY POINTER
            return;
        }
        p->statements = new_block;
    }

    p->statements[p->statement_count++] = statement;
}



/**
 * @brief Stores a block as a list of statements
 * 
 * statements are enclosed in { ... } and under a "node block"
 * 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_block_node(int line) {
    AstNodeBlock* node = (AstNodeBlock*)malloc(sizeof(AstNodeBlock));
    if (!node) return NULL;
    node->node.type = NODE_BLOCK;
    node->node.line = line;
    node->statement_count = 0;
    node->capacity = 4;
    node->statements = (AstNode**)malloc(sizeof(AstNode*) * node->capacity);
    return (AstNode*)node;
}

/**
 * @brief helper function to add a statement to a given block
 * 
 * @param block_node Pointer to the ast block node
 * @param statement pointer to the statement node to be added to the block
 */
void block_add_statement(AstNode* block_node, AstNode* statement) {
    AstNodeBlock* b = (AstNodeBlock*)block_node;
    if (b->statement_count >= b->capacity) {
        b->capacity *= 2;
        b->statements = (AstNode**)realloc(b->statements, sizeof(AstNode*) * b->capacity);
    }
    b->statements[b->statement_count++] = statement;
}

/**
 * @brief Creates a new Variable Declaration AST node.
 * 
 * @param name 
 * @param initializer 
 * @return AstNode* 
 */
AstNode* new_var_decl_node(Token name, AstNode* initializer, int line) {
    AstNodeVarDecl* node = (AstNodeVarDecl*)malloc(sizeof(AstNodeVarDecl));
    if (!node) return NULL;

    node->node.type = NODE_VAR_DECL;
    node->node.line = line;
    node->name = name;
    node->init = initializer;

    return (AstNode*)node;
}

/**
 * @brief Creates a new variable access AST Node
 * 
 * @param name 
 * @return AstNode* 
 */
AstNode* new_var_access_node(Token name, int line) {
    AstNodeVarAccess* node = (AstNodeVarAccess*)malloc(sizeof(AstNodeVarAccess));
    if (!node) return NULL;

    node->node.type = NODE_VAR_ACCESS;
    node->node.line = line;
    node->name = name;

    return (AstNode*)node;
}

/**
 * @brief Creates a new Print statement AST Node
 * 
 * @param expression expression node whose value should be printed
 * @return AstNode* pointer to the newly allocated AstNode, or NULL on allocation failure
 */
AstNode* new_print_stmt_node(AstNode* expression, int line) {
    AstNodePrintStmt* node = (AstNodePrintStmt*)malloc(sizeof(AstNodePrintStmt));
    if (!node) return NULL;

    node->node.type = NODE_PRINT_STMT;
    node->node.line = line;
    node->expression = expression;

    return (AstNode*)node;
}

/**
 * @brief Creates a new Integer Literal AST node
 */
AstNode* new_int_literal_node(int value, int line) {
    AstNodeIntLiteral* node = (AstNodeIntLiteral*)malloc(sizeof(AstNodeIntLiteral));
    if (node == NULL) return NULL;
    
    node->node.type = NODE_INT_LITERAL;
    node->node.line = line;
    node->value = value;

    return (AstNode*)node;
}

/**
 * @brief Creates a new string literal node
 * 
 * @param value 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_string_literal_node(char* value, int line) {
    AstNodeStringLiteral* node = (AstNodeStringLiteral*)malloc(sizeof(AstNodeStringLiteral));
    if (!node) return NULL;
    node->node.type = NODE_STRING_LITERAL;
    node->node.line = line;
    node->value = value;
    return (AstNode*)node;
}


AstNode* new_bool_literal_node(int value, int line) {
    AstNodeBoolLiteral* node = (AstNodeBoolLiteral*)malloc(sizeof(AstNodeBoolLiteral));
    if (!node) return NULL;
    node->node.type = NODE_BOOL_LITERAL;
    node->node.line = line;
    node->value = value;
    return (AstNode*)node;
}


AstNode* new_expr_stmt_node(AstNode* expression, int line) {
    AstNodeExprStmt* node = (AstNodeExprStmt*)malloc(sizeof(AstNodeExprStmt));
    if (!node) return NULL;

    node->node.type = NODE_EXPR_STMT;
    node->node.line = line;
    node->expression = expression;

    return (AstNode*)node;
}

/**
 * @brief Creates a new unary operator AST node
 */
AstNode* new_unary_op_node(Token op, AstNode* operand, int line) {
    AstNodeUnaryOp* node = (AstNodeUnaryOp*)malloc(sizeof(AstNodeUnaryOp));
    if (!node) return NULL;
    node->node.type = NODE_UNARY_OP;
    node->node.line = line;
    node->op = op;
    node->operand = operand;
    return (AstNode*)node;
}

/**
 * @brief Creates a new Binary Operator AST node
 */
AstNode* new_binary_op_node(Token op, AstNode* left, AstNode* right, int line) {
    AstNodeBinaryOp* node = (AstNodeBinaryOp*)malloc(sizeof(AstNodeBinaryOp));
    if (node == NULL) return NULL;

    node->node.type = NODE_BINARY_OP;
    node->node.line = line;
    node->op = op;
    node->left = left;
    node->right = right;

    return (AstNode*)node;
}

/**
 * @brief Creates a new Variable Assignment AST node
 */
AstNode* new_var_assign_node(Token name, AstNode* expression, int line) {
    AstNodeVarAssign* node = (AstNodeVarAssign*)malloc(sizeof(AstNodeVarAssign));
    if (!node) return NULL;

    node->node.type = NODE_VAR_ASSIGN;
    node->node.line = line;
    node->name = name;
    node->expression = expression;

    return (AstNode*)node;
}

/**
 * @brief Creates a new IF conditional branching node
 * 
 * @param condition 
 * @param thenBranch 
 * @param elseBranch 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_if_node(AstNode* condition, AstNode* thenBranch, AstNode* elseBranch, int line) {
    AstNodeIf* node = (AstNodeIf*)malloc(sizeof(AstNodeIf));
    if (!node) return NULL;
    node->node.type = NODE_IF;
    node->node.line = line;
    node->condition = condition;
    node->thenBranch = thenBranch;
    node->elseBranch = elseBranch;
    return (AstNode*)node;
}

/**
 * @brief Function to create a new WHILE condition node
 * 
 * @param condition 
 * @param body 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_while_node(AstNode* condition, AstNode* body, int line) {
    AstNodeWhile* node = (AstNodeWhile*)malloc(sizeof(AstNodeWhile));
    if (!node) return NULL;
    node->node.type = NODE_WHILE;
    node->node.line = line;
    node->condition = condition;
    node->body = body;
    return (AstNode*)node;
}


// --- Function related suite  ---

/**
 * @brief Function to declare a new function (lol) AST node
 * 
 * @param name Name of the function
 * @param params Pointer to the array of parameters used in the function
 * @param param_count Number of parameters passed to the function
 * @param returnType Return type of the function
 * @param body Body (block) of the function
 * @param line 
 * @return AstNode* 
 */
AstNode* new_func_decl_node(Token name, Token* params, int param_count, DataType returnType, AstNode* body, int line) {
    AstNodeFuncDecl* node = (AstNodeFuncDecl*)malloc(sizeof(AstNodeFuncDecl));
    if (!node) return NULL;
    node->node.type = NODE_FUNC_DECL;
    node->node.line = line;
    node->name = name;
    node->params = params; // Assumes caller allocated array
    node->param_count = param_count;
    node->returnType = returnType;
    node->body = body;
    return (AstNode*)node;
}

/**
 * @brief Function to create a new Return Datatype AST node
 * 
 * @param value Value of the return type
 * @param line 
 * @return AstNode* 
 */
AstNode* new_return_node(AstNode* value, int line) {
    AstNodeReturn* node = (AstNodeReturn*)malloc(sizeof(AstNodeReturn));
    if (!node) return NULL;
    node->node.type = NODE_RETURN;
    node->node.line = line;
    node->value = value;
    return (AstNode*)node;
}

/**
 * @brief Function to create a new function call AST node
 * 
 * @param callee Token which holds the callee of the function (block)
 * @param args Double Pointer to the Array holding the Arguments passed to the function
 * @param arg_count Number of arguments passed to the function
 * @param line 
 * @return AstNode* 
 */
AstNode* new_call_node(Token callee, AstNode** args, int arg_count, int line) {
    AstNodeCall* node = (AstNodeCall*)malloc(sizeof(AstNodeCall));
    if (!node) return NULL;
    node->node.type = NODE_CALL;
    node->node.line = line;
    node->callee = callee;
    node->args = args; // Assumes caller allocated array
    node->arg_count = arg_count;
    return (AstNode*)node;
}