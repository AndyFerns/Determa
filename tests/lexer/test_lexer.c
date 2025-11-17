/**
 * @file test_lexer.c
 * @brief Unit tests for the Determa Lexer.
 */

#include "test_lexer.h"
#include "test.h"     // For the CHECK macro
#include "lexer.h"    // The module we are testing
#include "token.h"    // For token types and helpers
#include <string.h>   // For strcmp

/**
 * @brief A helper to check a token's properties.
 */
static void check_token(Token token, TokenType type, const char* lexeme, int line) {
    char msg[256];
    
    sprintf(msg, "Type mismatch. Expected %s, Got %s", token_type_to_string(type), token_type_to_string(token.type));
    CHECK(token.type == type, msg);

    sprintf(msg, "Line mismatch. Expected %d, Got %d", line, token.line);
    CHECK(token.line == line, msg);

    sprintf(msg, "Lexeme mismatch. Expected '%s', Got '%.*s'", lexeme, token.length, token.lexeme);
    CHECK(strncmp(token.lexeme, lexeme, token.length) == 0 && token.length == strlen(lexeme), msg);
}


/**
 * Test a comprehensive stream of tokens.
 */
void test_all_tokens() {
    // This source string is designed to test every token type
    const char* source = "var x = 10; \n print (x + 1) / 2 * 3 - 4; // EOL";
    
    Lexer lexer = init_lexer(source);

    // Define the exact sequence of tokens we expect
    check_token(get_next_token(&lexer), TOKEN_VAR,       "var",    1);
    check_token(get_next_token(&lexer), TOKEN_ID,        "x",      1);
    check_token(get_next_token(&lexer), TOKEN_EQUALS,    "=",      1);
    check_token(get_next_token(&lexer), TOKEN_INT,       "10",     1);
    check_token(get_next_token(&lexer), TOKEN_SEMICOLON, ";",      1);
    
    check_token(get_next_token(&lexer), TOKEN_PRINT,     "print",  2);
    check_token(get_next_token(&lexer), TOKEN_LPAREN,    "(",      2);
    check_token(get_next_token(&lexer), TOKEN_ID,        "x",      2);
    check_token(get_next_token(&lexer), TOKEN_PLUS,      "+",      2);
    check_token(get_next_token(&lexer), TOKEN_INT,       "1",      2);
    check_token(get_next_token(&lexer), TOKEN_RPAREN,    ")",      2);
    check_token(get_next_token(&lexer), TOKEN_SLASH,     "/",      2);
    check_token(get_next_token(&lexer), TOKEN_INT,       "2",      2);
    check_token(get_next_token(&lexer), TOKEN_STAR,      "*",      2);
    check_token(get_next_token(&lexer), TOKEN_INT,       "3",      2);
    check_token(get_next_token(&lexer), TOKEN_MINUS,     "-",      2);
    check_token(get_next_token(&lexer), TOKEN_INT,       "4",      2);
    check_token(get_next_token(&lexer), TOKEN_SEMICOLON, ";",      2);
    
    check_token(get_next_token(&lexer), TOKEN_EOF,       "",       2);
}

/**
 * Test that line numbers are incremented correctly.
 */
void test_line_numbers() {
    const char* source = "a\nb\n\nc\n";
    Lexer lexer = init_lexer(source);
    
    check_token(get_next_token(&lexer), TOKEN_ID,  "a", 1);
    check_token(get_next_token(&lexer), TOKEN_ID,  "b", 2);
    // The empty line should be skipped, and 'c' should be on line 4
    check_token(get_next_token(&lexer), TOKEN_ID,  "c", 4);
    check_token(get_next_token(&lexer), TOKEN_EOF, "",  4);
}

/**
 * Test that whitespace (spaces, tabs) and comments are skipped.
 */
void test_whitespace_and_comments() {
    const char* source = " ( \t var // This is a comment \n ) ";
    Lexer lexer = init_lexer(source);

    // The lexer should skip the space and tab
    check_token(get_next_token(&lexer), TOKEN_LPAREN, "(", 1);
    // The lexer should skip the space, tab, and entire comment
    check_token(get_next_token(&lexer), TOKEN_VAR, "var", 1);
    // The lexer should skip the newline and space
    check_token(get_next_token(&lexer), TOKEN_RPAREN, ")", 2);
    // The lexer should skip the final space
    check_token(get_next_token(&lexer), TOKEN_EOF, "", 2);
}