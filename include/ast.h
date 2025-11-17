/**
 * @file ast.h
 * @author Andrew Fernandes
 * @brief Defines the data structures for the Abstract Syntax Tree (AST)
 * 
 * The parser will create a tree of these nodes to represent
 * the program's structure
 * 
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef AST_H
#define AST_H

#include "token.h"

/**
 * @enum AstNodeType
 * @brief An enumeration of all possible AST node types
 */
typedef enum {
    NODE_INT_LITERAL,  // A simple integer
    NODE_BINARY_OP,    // An operation like 1 + 2
    NODE_VAR_DECL,     // var x = 10;
    NODE_VAR_ASSIGN,   // x = 20;
    NODE_VAR_ACCESS,   // print x;
    NODE_PRINT_STMT,   // print 10;
    // We will add more as we go (if, while, funcs)
} AstNodeType;

/**
 * @struct AstNode
 * @brief The base struct for all AST nodes
 *
 * This is the "base class"
 * We will cast this to more specific node types
 */
typedef struct AstNode {
    AstNodeType type;
} AstNode;


// --- Specific Node Structs ---
// These structs "inherit" from AstNode

/**
 * @struct AstNodeIntLiteral
 * @brief Represents an integer literal (e.g., 123)
 */
typedef struct {
    AstNode node; // Base "class"
    int value;
} AstNodeIntLiteral;

/**
 * @struct AstNodeBinaryOp
 * @brief Represents a binary operation (e.g., left + right)
 */
typedef struct {
    AstNode node;      // Base "class"
    Token op;          // The operator token (TOKEN_PLUS, etc)
    AstNode* left;     // Left-hand side expression
    AstNode* right;    // Right-hand side expression
} AstNodeBinaryOp;

/**
 * @struct AstNodeVarDecl
 * @brief Represents a variable declaration (e.g., var x = 10;)
 */
typedef struct {
    AstNode node;      // Base "class"
    Token name;        // The identifier token for the variable
    AstNode* init;     // The initializer expression (can be NULL)
} AstNodeVarDecl;

// We will add more node types here (Assign, Access, etc)


// --- Helper Functions ---

/**
 * @brief Creates a new Integer Literal AST node
 * @param value The integer value
 * @return A pointer to the new AstNode
 */
AstNode* new_int_literal_node(int value);

/**
 * @brief Creates a new Binary Operator AST node
 * @param op The operator token
 * @param left The left-hand side expression
 * @param right The right-hand side expression
 * @return A pointer to the new AstNode
 */
AstNode* new_binary_op_node(Token op, AstNode* left, AstNode* right);


/**
 * @brief Frees an entire AST tree recursively
 * @param node The root node of the tree to free
 */
void free_ast(AstNode* node);

/**
 * @brief Prints an AST tree to the console (for debugging)
 * @param node The root node of the tree to print
 */
void print_ast(AstNode* node);


#endif // AST_H