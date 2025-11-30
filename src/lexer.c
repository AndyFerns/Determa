/**
 * @file lexer.c
 * @author Andrew Fernandes
 * @brief Implementation of the Determa Lexer (Scanner).
 * 
 * This is a hand-written DFA. The get_next_token() function
 * acts as the main "simulator" loop, and the switch statements
 * act as the DFA's transition table.
 * 
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "lexer.h"

#include <string.h> // For memcmp, strlen
#include <ctype.h>  // For isalpha, isdigit

/**
 * @brief Initializes a new Lexer.
 */
Lexer init_lexer(const char* source) {
    Lexer lexer;
    lexer.source = source;
    lexer.start = 0;
    lexer.current = 0;
    lexer.line = 1;
    return lexer;
}

// --- Helper Functions ---

/**
 * @brief Checks if the lexer has reached the end of the source.
 */
static int is_at_end(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
}

/**
 * @brief Consumes and returns the current character, advancing the lexer.
 */
static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->source[lexer->current - 1];
}

/**
 * @brief Looks at the current character without consuming it.
 */
static char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

/**
 * @brief Looks at the character after the current one.
 */
static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) {
        return '\0';
    }
    return lexer->source[lexer->current + 1];
}

/**
 * @brief Helper to create a token of a given type.
 */
static Token make_token(Lexer* lexer, TokenType type) {
    return create_token(type, lexer->source + lexer->start,
                        lexer->current - lexer->start, lexer->line);
}

/**
 * @brief Helper to create an error token.
 */
static Token error_token(Lexer* lexer, const char* message) {
    return create_token(TOKEN_ERROR, message, (int)strlen(message), lexer->line);
}

/**
 * @brief Skips whitespace and comments.
 * This is part of the DFA's "start" state logic.
 */
static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n': // Newline
                lexer->line++;
                advance(lexer);
                break;
            case '/': // A comment?
                if (peek_next(lexer) == '/') {
                    // A single-line comment. Consume until the end of the line.
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else {
                    // Not a comment, just a slash. Return.
                    return;
                }
                break;
            default:
                return; // Not whitespace
        }
    }
}

/**
 * @brief Helper to check if a lexeme is a keyword.
 */
static TokenType check_keyword(Lexer* lexer, int start, int length,
                               const char* rest, TokenType type) {
    // Check if the length matches and the string matches
    if (lexer->current - lexer->start == start + length &&
        memcmp(lexer->source + lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_ID; // Not a keyword, just a regular identifier
}

/**
 * @brief Reads the rest of an identifier.
 */
static TokenType identifier_type(Lexer* lexer) {
    // This is a simple state machine (a "Trie")
    // for checking keywords.
    switch (lexer->source[lexer->start]) {
        case 'p':
            return check_keyword(lexer, 1, 4, "rint", TOKEN_PRINT);
        case 'v':
            return check_keyword(lexer, 1, 2, "ar", TOKEN_VAR);
    }
    return TOKEN_ID; // Default to identifier
}

/**
 * @brief Consumes an identifier token.
 */
static Token read_identifier(Lexer* lexer) {
    // Consume all alphanumeric characters (and _)
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    // Check if the identifier is actually a keyword
    return make_token(lexer, identifier_type(lexer));
}

/**
 * @brief Consumes a number (integer) token.
 */
static Token read_number(Lexer* lexer) {
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }
    return make_token(lexer, TOKEN_INT);
}


/**
 * @brief Function to implement string scanning logic 
 * 
 * @param lexer 
 * @return Token 
 */
static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string.");
    }

    // The closing quote.
    advance(lexer);
    return make_token(lexer, TOKEN_STRING);
}


// --- Main Lexer Function ---

/**
 * @brief Scans and returns the next token.
 * This is the main "run" function of our DFA.
 */
Token get_next_token(Lexer* lexer) {
    // 1. Skip whitespace and comments
    skip_whitespace(lexer);

    // 2. Mark the start of the new token
    lexer->start = lexer->current;

    // 3. Check for End-Of-File
    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }

    // 4. This is the main transition table (DFA)
    char c = advance(lexer);

    // Check for identifiers and keywords
    if (isalpha(c) || c == '_') {
        return read_identifier(lexer);
    }
    // Check for numbers
    if (isdigit(c)) {
        return read_number(lexer);
    }

    // Check for double quote 
    if (c == '"') {
        return string(lexer);
    }

    // Check for single-character tokens
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case '=': return make_token(lexer, TOKEN_EQUALS);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '-': return make_token(lexer, TOKEN_MINUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
    }

    // 5. If no rule matched, it's an unrecognized character.
    return error_token(lexer, "Unexpected character.");
}