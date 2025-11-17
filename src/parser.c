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

// --- Parser Globals (State) ---

// We will build this out soon
// typedef struct {
//     Lexer lexer;
//     Token current;
//     Token previous;
//     int had_error;
// } Parser;

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
