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
 * © 2025
 */

#ifndef AST_H
#define AST_H

#include "token.h"

/**
 * @enum AstNodeType
 * @brief An enumeration of all possible AST node types
 */
typedef enum {
    NODE_PROGRAM,           // Root Node, holds a list of statements 
    NODE_INT_LITERAL,       // A simple integer
    NODE_STRING_LITERAL,    // Represents a string literal
    NODE_UNARY_OP,          // An operation like -5
    NODE_BINARY_OP,         // An operation like 1 + 2
    NODE_VAR_DECL,          // var x = 10;
    NODE_VAR_ASSIGN,        // x = 20;
    NODE_VAR_ACCESS,        // x;
    NODE_PRINT_STMT,        // print 10;
    NODE_EXPR_STMT          // 1 + 1; Expression as a statements
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
    int line;           // Line number for debugging information
} AstNode;


// --- Specific Node Structs ---
// These structs "inherit" from AstNode

/**
 * @struct AstNodeProgram
 * @brief The root node representing the entire program
 */
typedef struct {
    AstNode node;
    AstNode** statements;   // Dynamic array of statement nodes
    int statement_count;    // Number of statements in the program
    int capacity;           // Required for dynamic array growth
} AstNodeProgram;

/**
 * @struct AstNodeIntLiteral
 * @brief Represents an integer literal (e.g., 123)
 */
typedef struct {
    AstNode node; // Base "class"
    int value;
} AstNodeIntLiteral;


/**
 * @brief Represents a string literal (e.g., "hello")
 * @struct AstNodeStringLiteral
 */
typedef struct {
    AstNode node;
    char* value;
} AstNodeStringLiteral;


typedef struct {
    AstNode node;       // Base class
    Token op;           // operation to be performed ('-')
    AstNode* operand;   // operand on which to perform the op
} AstNodeUnaryOp;

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

/**
 * @struct AstNodeVarAssign
 * @brief Represents assigning of an expression to a variable 
 * 
 * (Eg. x = 10;)
 */
typedef struct {
    AstNode node;      // Base "class"
    Token name;        // The identifier token for the variable
    AstNode* expression;
} AstNodeVarAssign;

/**
 * @struct AstNodeVarAccess
 * @brief Represents reading a variable 'name'
 */
typedef struct {
    AstNode node;
    Token name;
} AstNodeVarAccess;

/**
 * @struct AstNodePrintStmt
 * @brief Represents 'print expression;'
 */
typedef struct {
    AstNode node;
    AstNode* expression;
} AstNodePrintStmt;

/**
 * @struct AstNodeExprStmt
 * @brief Represents an expression used as a statement
 */
typedef struct {
    AstNode node;
    AstNode* expression;
} AstNodeExprStmt;


// ========================
// --- Helper Functions ---
// ========================

AstNode* new_program_node(AstNode** statements, int count);

/**
 * @brief Adds a statement to the Program node
 */
void program_add_statement(AstNode* program_node, AstNode* statement);

/**
 * @brief Creates a new Variable Declaration AST node.
 *
 * @param name identifier token representing the variable name
 * @param initializer
 * @param line Line number for debugging
 */
AstNode* new_var_decl_node(Token name, AstNode* initializer, int line);

/**
 * @brief Creates a new Variable Access AST node.
 */
AstNode* new_var_access_node(Token name, int line);

/**
 * @brief Creates a new Print Statement AST node.
 */
AstNode* new_print_stmt_node(AstNode* expression, int line);

/**
 * @brief Creates a new Expression Statement AST node.
 */
AstNode* new_expr_stmt_node(AstNode* expression, int line);

/**
 * @brief Creates a new Integer Literal AST node
 */
AstNode* new_int_literal_node(int value, int line);


/**
 * @brief Creates a new String literal AST node
 */
AstNode* new_string_literal_node(char* value, int line);

/**
 * @brief Creates a new Binary Operator AST node
 */
AstNode* new_binary_op_node(Token op, AstNode* left, AstNode* right, int line);


/**
 * @brief Creates a new Unary Operator AST node
 */
AstNode* new_unary_op_node(Token op, AstNode* operand, int line);

/**
 * @brief Creates a new Variable Assignment AST node
 */
AstNode* new_var_assign_node(Token name, AstNode* expression, int line);

/**
 * @brief Frees an entire AST tree recursively
 */
void free_ast(AstNode* node);

/**
 * @brief Prints an AST tree
 */
void print_ast(AstNode* node);

#endif
