/**
 * @file token.h
 * @author Andrew Fernandes
 * @brief Defines the Token data structures for the Determa language.
 * 
 * This file contains the TokenType enum, the Token struct,
 * and helper functions for creating and printing tokens.
 * 
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>


/**
 * @enum TokenType
 * @brief An enumeration of all token types recognized by the Determa lexer.
 */
typedef enum {
    // Single-character tokens
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )

    // Arithmetic Operators and 
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_STAR,       // *
    TOKEN_SLASH,      // /
    TOKEN_SEMICOLON,  // ;
    TOKEN_EQUALS,     // =

    // Literals
    TOKEN_INT,        // 123
    TOKEN_ID,         // x, my_var

    // Keywords
    TOKEN_VAR,        // var
    TOKEN_PRINT,      // print

    // Control tokens
    TOKEN_ERROR,      // Represents a lexing error
    TOKEN_EOF         // End Of File
} TokenType;

/**
 * @struct Token
 * @brief Represents a single token (lexeme) from the source code.
 *
 * @var Token::type
 * The type of the token (e.g., TOKEN_INT).
 * @var Token::lexeme
 * A pointer to the actual string in the source code.
 * @var Token::length
 * The length of the lexeme.
 * @var Token::line
 * The line number where the token appears, for error reporting.
 */
typedef struct {
    TokenType type;
    const char* lexeme;
    int length;
    int line;
} Token;

#endif