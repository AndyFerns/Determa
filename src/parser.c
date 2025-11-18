/**
 * @file parser.c
 * @brief Implementation of the Determa Parser
 *
 * This is a recursive descent parser that builds the AST
 */

#include "parser.h"
#include "ast.h"
#include "token.h"
#include "lexer.h"
#include "debug.h" // for PDA Logging

#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy


// --- Globals for PDA Debugger ---
int pda_debug_enabled = 0;
int pda_debug_indent = 0;

// --- Parser Globals (State) ---

/**
 * @struct Parser
 * @brief Holds the state of the parser as it consumes tokens
 *
 * This is the practical implementation of our "PDA".
 * The C call stack acts as the PDA's stack.
 */
typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int had_error;
    // We will add a 'panic_mode' flag later for error recovery
} Parser;

// --- Forward Declarations for recursive functions ---
static AstNode* parse_expression(Parser* parser);
static AstNode* parse_term(Parser* parser);
static AstNode* parse_factor(Parser* parser);
static AstNode* parse_primary(Parser* parser);
static AstNode* parse_declaration(Parser* parser);
static AstNode* parse_statement(Parser* parser);

// --- Error Handling & Token Helpers ---

/**
 * @brief Prints a parser error message
 */
static void error_at_current(Parser* parser, const char* message) {
    parser->had_error = 1;
    // Don't print if we've already hit an error
    // (This prevents a cascade of errors)
    // We will implement panic_mode later
    
    printf("[Line %d] Error", parser->current.line);
    if (parser->current.type == TOKEN_EOF) {
        printf(" at end");
    } else if (parser->current.type != TOKEN_ERROR) {
        printf(" at '%.*s'", parser->current.length, parser->current.lexeme);
    }
    printf(": %s\n", message);
}

/**
 * @brief Consumes the next token from the lexer
 */
static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    // Keep scanning until we get a non-error token
    for (;;) {
        parser->current = get_next_token(&parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        // Report the lexer error
        error_at_current(parser, parser->current.lexeme);
    }
}

/**
 * @brief Consumes the current token if it matches the expected type
 *
 * If it matches, advance to the next token
 * If not, report an error
 */
static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    error_at_current(parser, message);
}

/**
 * @brief Checks if the current token is of a given type
 */
static int check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

/**
 * @brief Checks if the current token matches any in a list
 *
 * This is a helper for binary operators
 */
static int match(Parser* parser, TokenType types[], int count) {
    for (int i = 0; i < count; i++) {
        if (check(parser, types[i])) {
            advance(parser);
            return 1;
        }
    }
    return 0;
}



// --- Parsing Functions (The Grammar) ---

/**
 * @brief Parses a "primary" expression (the lowest level)
 *
 * Grammar Rule:
 * primary -> NUMBER | "(" expression ")"
 */
static AstNode* parse_primary(Parser* parser) {
    TRACE_ENTER("Primary");
    // It's a number literal
    if (check(parser, TOKEN_INT)) {
        char buf[64];
        int len = parser->current.length;
        if (len >= 63) len = 63; // Truncate if too long

        memcpy(buf, parser->current.lexeme, len);
        buf[len] = '\0';

        long val = strtol(buf, NULL, 10);
        advance(parser);
        TRACE_EXIT("Primary (IntLiteral)");
        return new_int_literal_node((int)val);
    }

    // Handle identifier
    if (check(parser, TOKEN_ID)) {
        Token name = parser->current;
        advance(parser);
        TRACE_EXIT("Primary (VarAccess)");
        return new_var_access_node(name);
    }

    // Consume '('
    if (check(parser, TOKEN_LPAREN)) {
        advance(parser); 
        AstNode* expr = parse_expression(parser); // Parse inner expression
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        TRACE_EXIT("Primary (Grouping)");
        return expr;
    }

    // If we get here, it's an error
    error_at_current(parser, "Expected expression");
    TRACE_EXIT("Primary (Error)");
    return NULL;
}

/**
 * @brief Parses "factor" expressions (multiplication, division)
 *
 * Grammar Rule:
 * factor -> primary ( ( "/" | "*" ) primary )*
 */
static AstNode* parse_factor(Parser* parser) {
    TRACE_ENTER("Factor");
    AstNode* node = parse_primary(parser);

    TokenType op_types[] = {TOKEN_STAR, TOKEN_SLASH};
    while (match(parser, op_types, 2)) {
        Token op = parser->previous;
        AstNode* right = parse_primary(parser);
        node = new_binary_op_node(op, node, right);
    }
    
    TRACE_EXIT("Factor");
    return node;
}

/**
 * @brief Parses "term" expressions (addition, subtraction)
 *
 * Grammar Rule:
 * term -> factor ( ( "-" | "+" ) factor )*
 */
static AstNode* parse_term(Parser* parser) {
    TRACE_ENTER("Term");
    AstNode* node = parse_factor(parser);

    TokenType op_types[] = {TOKEN_PLUS, TOKEN_MINUS};
    while (match(parser, op_types, 2)) {
        Token op = parser->previous;
        AstNode* right = parse_factor(parser);
        node = new_binary_op_node(op, node, right);
    }
    
    TRACE_EXIT("Term");
    return node;
}


/**
 * @brief Parses an expression (Handles precedence)
 *
 * Grammar Rule:
 * expression -> term
 */
static AstNode* parse_expression(Parser* parser) {
    TRACE_ENTER("Expression");
    // This is the entry point for the precedence stack
    AstNode* node = parse_term(parser);
    TRACE_EXIT("Expression");
    return node;
}

// --- Statement Parsing ---

/**
 * @brief Parses a print statement
 * 
 * Grammar:
 * @code
 print_statement → "print" expression ";"
 @endcode
 * 
 * @note assumes the 'print' keyword has already been consumed
 * by the caller. It parses the following expression, ensures that the
 * statement ends with a semicolon, and returns a fully constructed
 * `AstNodePrintStmt` node.
 *
 * PDA tracing is emitted when debug mode is enabled
 * 
 * @param parser Pointer to the parser instance
 * @return AstNode* new print-statement AST node
 */
static AstNode* parse_print_statement(Parser* parser) {
    TRACE_ENTER("PrintStmt");
    AstNode* value = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after value");
    TRACE_EXIT("PrintStmt");
    return new_print_stmt_node(value);
}

/**
 * @brief Parses a variable declaration
 * 
 * @details Grammar:
 * @code
 * var_declaration → IDENTIFIER ( "=" expression )? ";"
 * @endcode
 * 
 * @param parser Pointer to the parser instance
 * @return AstNode* variable-declaration AST node
 */
static AstNode* parse_var_declaration(Parser* parser) {
    TRACE_ENTER("VarDecl");
    consume(parser, TOKEN_ID, "Expected variable name");
    Token name = parser->previous;
    
    AstNode* initializer = NULL;
    if (match(parser, (TokenType[]){TOKEN_EQUALS}, 1)) {
        initializer = parse_expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    TRACE_EXIT("VarDecl");
    return new_var_decl_node(name, initializer);
}

/**
 * @brief Handles executable statements (non-declaration)
 * 
 * eg: 
 * @code 
    print x + 1;
    x + y * 2;
    a = b + c;   // If assignment handled later
   @endcode
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_statement(Parser* parser) {
    TRACE_ENTER("Statement");
    if (match(parser, (TokenType[]){TOKEN_PRINT}, 1)) {
        AstNode* stmt = parse_print_statement(parser);
        TRACE_EXIT("Statement (Print)");
        return stmt;
    }
    
    // If it's not a print statement, assume it's an expression statement
    AstNode* expr = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    TRACE_EXIT("Statement (Expr)");
    return new_expr_stmt_node(expr); 
}

/**
 * @brief The top-level controller
 * 
 * It decides:
 * If the next token is var → call parse_var_declaration
 * Otherwise → treat it as a statement and call parse_statement
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_declaration(Parser* parser) {
    TRACE_ENTER("Declaration");
    if (match(parser, (TokenType[]){TOKEN_VAR}, 1)) {
        AstNode* decl = parse_var_declaration(parser);
        TRACE_EXIT("Declaration (Var)");
        return decl;
    }
    AstNode* stmt = parse_statement(parser);
    TRACE_EXIT("Declaration (Stmt)");
    return stmt;
}

/**
 * @brief The main function to parse source code
 *
 * This function initializes a parser and begins the parsing process
 *
 * @param source The source code string to parse
 * @param pda_debug_mode 0 to run silently, 1 to enable PDA trace logging
 * 
 * @return AstNode* The root of the generated AST (or NULL on error)
 */
AstNode* parse(const char* source, int pda_debug_mode) {
    /* ---- Debug Mode Setup ---- */
    pda_debug_enabled = pda_debug_mode;
    pda_debug_indent  = 0;

    /* ---- Init Parser State ---- */
    Parser parser;
    parser.lexer     = init_lexer(source);
    parser.had_error = 0;

    advance(&parser);     // Load first token

    // Create Program Node 
    AstNode* program = new_program_node(NULL, 0);

    if (!program) {
        fprintf(stderr, "Fatal: Failed to allocate program node.\n");
        return NULL;
    }

    // Parse Until EOF 
    while (!check(&parser, TOKEN_EOF)) {

        AstNode* stmt = parse_declaration(&parser);

        if (stmt != NULL) {
            program_add_statement(program, stmt);
        } else {
            // For now, if we hit an error, we might loop infinitely if we don't consume tokens.
            // A simple sync is to advance until semicolon.
            if (parser.had_error) {
                 // Simple panic mode recovery: skip until next statement
                 while (!check(&parser, TOKEN_EOF) && !check(&parser, TOKEN_SEMICOLON)) {
                     advance(&parser);
                 }
                 if (check(&parser, TOKEN_SEMICOLON)) advance(&parser);
                 parser.had_error = 0; // Reset error to try parsing next statement
            } else {
                break; 
            }
        }
    }

    // Final Error Check 
    if (parser.had_error) {
        free_ast(program);   // Clean up partial tree
        return NULL;
    }

    return program;  // Success
}