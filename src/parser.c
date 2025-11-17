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

#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy

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
 * @brief The main function to parse source code
 *
 * (STUB) This is a placeholder
 */
AstNode* parse(const char* source) {
    // (void)source; // Suppress unused warning for now
    
    // Test the lexer
    Lexer lexer = init_lexer(source);
    printf("Parser: Lexer initialized. First token:\n");
    print_token(get_next_token(&lexer));
    
    // For now, we don't return a real AST
    return NULL;
}
