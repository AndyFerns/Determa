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
#include "types.h"

/**
 * @enum AstNodeType
 * @brief An enumeration of all possible AST node types
 */
typedef enum {
    NODE_PROGRAM,           // Root Node, holds a list of statements 
    NODE_BLOCK,             // A set of statements inside { ... } Curly braces

    NODE_INT_LITERAL,       // A simple integer
    NODE_STRING_LITERAL,    // Represents a string literal
    NODE_BOOL_LITERAL,      // Represents a boolean literal

    NODE_UNARY_OP,          // An operation like -5
    NODE_BINARY_OP,         // An operation like 1 + 2

    NODE_VAR_DECL,          // var x = 10;
    NODE_VAR_ASSIGN,        // x = 20;
    NODE_VAR_ACCESS,        // x;

    NODE_PRINT_STMT,        // print 10;
    NODE_EXPR_STMT,         // 1 + 1; Expression as a statements

    NODE_IF,                // if conditional block
    NODE_WHILE,             // while conditional block

    // Functions
    NODE_FUNC_DECL,         // Function definition keyword
    NODE_RETURN,            // Function return value keyword
    NODE_CALL               // function call inside blocks
    // add more in the future
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
 * @struct AstNodeBlock
 * @brief Represents a block of statements: { ... }
 */
typedef struct {
    AstNode node;
    AstNode** statements;
    int statement_count;
    int capacity;
} AstNodeBlock;


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
    char* value; // Null-terminated string content (quotes stripped)
} AstNodeStringLiteral;


/**
 * @brief Represents a boolean literal (eg. True False)
 * 
 */
typedef struct {
    AstNode node;
    int value;  // 1 for true 0 for false
} AstNodeBoolLiteral;

/**
 * @brief Represents a unary operator (eg -5, -int)
 * 
 */
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
    Token name;        // The identifier token for the variable being updated
    AstNode* expression;// the new value
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


/**
 * @struct AstNodeIf
 * @brief Represents if/else logic
 */
typedef struct {
    AstNode node;
    AstNode* condition;
    AstNode* thenBranch; // Must be a NODE_BLOCK
    AstNode* elseBranch; // Can be NODE_BLOCK (else) or NODE_IF (elif), or NULL
} AstNodeIf;


/**
 * @struct AstNodeWhile
 * @brief Represents a while loop
 */
typedef struct {
    AstNode node;
    AstNode* condition;
    AstNode* body; // Must be a NODE_BLOCK
} AstNodeWhile;


/**
 * @struct AstNodeFuncDecl
 * 
 * @brief Represents: func name(arg1, arg2) : type { body }
 */
typedef struct {
    AstNode node;
    AstNode* body;      // NODE_BLOCK
    Token name;         // Name of the function
    Token* params;      // Array of parameter names
    int param_count;    // count of the number of parameters
    DataType returnType;// data type of the return type
} AstNodeFuncDecl;

/**
 * @struct AstNodeReturn
 * @brief Represents: return expr;
 */
typedef struct {
    AstNode node;
    AstNode* value; // Can be NULL for void return
} AstNodeReturn;


/**
 * @struct AstNodeCall
 * @brief Represents: name(arg1, arg2)
 */
typedef struct {
    AstNode node;
    Token callee;       // Name of function being called
    AstNode** args;     // Array of arguments
    int arg_count;
} AstNodeCall;


// ========================
// --- Helper Functions ---
// ========================


/**
 * @brief Creates a new Program AST Node
 * 
 * @param statements 
 * @param count 
 * @return AstNode* 
 */
AstNode* new_program_node(AstNode** statements, int count);


/**
 * @brief Adds a statement to the Program node
 */
void program_add_statement(AstNode* program_node, AstNode* statement);

/**
 * @brief Creates a new block AST node
 * 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_block_node(int line);


/**
 * @brief Adds a statement to a given block node
 * 
 * A block is defined as a set of statements enclosed in { ... } curly braces
 * 
 * @param block_node 
 * @param statement 
 */
void block_add_statement(AstNode* block_node, AstNode* statement);


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
 * @brief Creates a new boolean literal ast node
 */
AstNode* new_bool_literal_node(int value, int line);

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
 * @brief Creates a new IF conditional block AST node
 * @return AstNode* 
 */
AstNode* new_if_node(AstNode* condition, AstNode* thenBranch, AstNode* elseBranch, int line);


/**
 * @brief Creates a new WHILE conditional block AST node
 * @return AstNode* 
 */
AstNode* new_while_node(AstNode* condition, AstNode* body, int line);

/**
 * @brief Function to declare a new function (lol) AST node
 * 
 * @return AstNode* 
 */
AstNode* new_func_decl_node(Token name, Token* params, int param_count, DataType returnType, AstNode* body, int line);


/**
 * @brief Function to create a new Return DataType AST node
 * 
 * @param value 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_return_node(AstNode* value, int line);

/**
 * @brief Function to create a new function call AST node
 * 
 * @param callee 
 * @param args 
 * @param arg_count 
 * @param line 
 * @return AstNode* 
 */
AstNode* new_call_node(Token callee, AstNode** args, int arg_count, int line);

/**
 * @brief Frees an entire AST tree recursively
 */
void free_ast(AstNode* node);

/**
 * @brief Prints an AST tree
 */
void print_ast(AstNode* node);

#endif
