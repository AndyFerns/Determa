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
    if (check(parser, TOKEN_INT)) {
        // It's a number literal
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

    if (check(parser, TOKEN_LPAREN)) {
        advance(parser); // Consume '('
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


/**
 * @brief The main function to parse source code
 */
AstNode* parse(const char* source, int pda_debug_mode) {
    // Set the debug mode
    pda_debug_enabled = pda_debug_mode;
    pda_debug_indent = 0;

    Parser parser;
    parser.lexer = init_lexer(source);
    parser.had_error = 0;
    
    // Prime the parser by loading the first token
    advance(&parser);
    
    // For now, we only parse a single expression
    AstNode* ast = parse_expression(&parser);
    
    // A simple program must end with a semicolon
    consume(&parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    
    // If we had an error, free the (partial) tree and return NULL
    if (parser.had_error) {
        free_ast(ast);
        return NULL;
    }

    return ast;
}