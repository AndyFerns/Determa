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
#include "token.h"

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
static TokenType check_keyword(Lexer* lexer, int start, int length, const char* rest, TokenType type) {
    // Check if the length matches and the string matches
    if (lexer->current - lexer->start == start + length && memcmp(lexer->source + lexer->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_ID; // Not a keyword, just a regular identifier
}

/**
 * @brief Reads the rest of an identifier. 
 * 
 * This is a simple state machine (a "Trie") for checking keywords.
 */
static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->source[lexer->start]) {
        // print
        case 'p':
            return check_keyword(lexer, 1, 4, "rint", TOKEN_PRINT);
        
        // var or void
        case 'v': {
            int length = lexer->current - lexer->start;
            if (length == 3) {
                return check_keyword(lexer, 1, 2, "ar", TOKEN_VAR);
            }

            if (length == 4) {
                return check_keyword(lexer, 1, 3, "oid", TOKEN_TYPE_VOID);
            }
            return TOKEN_ID;
        }

        // true
        case 't':
            return check_keyword(lexer, 1, 3, "rue", TOKEN_TRUE);
        
        // false or func
        case 'f': {
            int length = lexer->current - lexer->start;
            if (length == 4) {
                return check_keyword(lexer, 1, 3, "unc", TOKEN_FUNC);
            }
            if (length == 5) {
                return check_keyword(lexer, 1, 4, "alse", TOKEN_FALSE);
            }
            return TOKEN_ID;
        }

        // if or int
        case 'i': {
            int length = lexer->current - lexer->start;
            if (length == 2) {
                return check_keyword(lexer, 1, 1, "f", TOKEN_IF);
            }

            if (length == 3) {
                return check_keyword(lexer, 1, 2,  "nt", TOKEN_TYPE_INT);
            }

            return TOKEN_ID;
        }
        
        // else or elif
        case 'e': {
            int length = lexer->current - lexer->start;
            if (length == 4) {
                if (memcmp(lexer->source + lexer->start, "elif", 4) == 0) {
                    return TOKEN_ELIF;
                }
                if (memcmp(lexer->source + lexer->start, "else", 4) == 0) {
                    return TOKEN_ELSE;
                }
            }
            return TOKEN_ID;
        }

        // while
        case 'w':
            return check_keyword(lexer, 1, 4, "hile", TOKEN_WHILE);
        // return
        case 'r':
            return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);

        // bool
        case 'b':
            return check_keyword(lexer, 1, 3, "ool", TOKEN_TYPE_BOOL);
        // str
        case 's':
            return check_keyword(lexer, 1, 2, "tr",TOKEN_TYPE_STRING);
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
        advance(lexer);
        return error_token(lexer, "Unterminated string.");
    }

    // The closing quote.
    advance(lexer);
    return make_token(lexer, TOKEN_STRING);
}


/**
 * @brief Helper to match the next character (for 2 character tokens)
 * 
 * @param lexer 
 * @param expected expected character to be matched
 * @return int returns 0 on failure to match the next char \n
 * returns 1 otherwise
 */
static int match_char(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return 0;
    if (lexer->source[lexer->current] != expected) return 0;
    lexer->current++;
    return 1;
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
        case '{': return make_token(lexer, TOKEN_LEFT_BRACE);  
        case '}': return make_token(lexer, TOKEN_RIGHT_BRACE); 

        case ',': return make_token(lexer, TOKEN_COMMA); 
        case ':': return make_token(lexer, TOKEN_COLON); 

        case ';': return make_token(lexer, TOKEN_SEMICOLON);

        // Using match_char to peek next char and consume if match
        case '+': return make_token(lexer, match_char(lexer, '=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
        case '-': return make_token(lexer, match_char(lexer, '=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
        case '*': return make_token(lexer, match_char(lexer, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
        case '/': return make_token(lexer, match_char(lexer, '=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
        case '%': return make_token(lexer, match_char(lexer, '=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
        
        case '!':
            return make_token(lexer, match_char(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(lexer, match_char(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUALS);
        case '<':
            return make_token(lexer, match_char(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return make_token(lexer, match_char(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    }

    // 5. If no rule matched, it's an unrecognized character.
    return error_token(lexer, "Unexpected character.");
}