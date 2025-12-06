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
    TOKEN_LEFT_BRACE, // { 
    TOKEN_RIGHT_BRACE,// }
    TOKEN_COMMA,      // , used as a separator in functions and arrays 
    TOKEN_COLON,      // : used for function return values

    // Arithmetic Operators 
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_STAR,       // *
    TOKEN_SLASH,      // /
    TOKEN_PERCENT,    // %
    TOKEN_SEMICOLON,  // ;
    TOKEN_EQUALS,     // = (assignment)

    // Conditional 1-2 character tokens
    TOKEN_BANG,         // ! (negation)
    TOKEN_BANG_EQUAL,  // != (not equals to)
    TOKEN_EQUAL_EQUAL,  // == (equivalence)
    TOKEN_LESS,         // <
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_GREATER,      // >
    TOKEN_GREATER_EQUAL,// >=

    // Literals
    TOKEN_INT,        // 123
    TOKEN_ID,         // x, my_var
    TOKEN_STRING,     // "hello", 'world'

    // Compound Assignment 
    TOKEN_PLUS_EQUAL,    // +=
    TOKEN_MINUS_EQUAL,   // -=
    TOKEN_STAR_EQUAL,    // *=
    TOKEN_SLASH_EQUAL,   // /=
    TOKEN_PERCENT_EQUAL, // %=

    // Keywords
    TOKEN_VAR,        // var
    TOKEN_PRINT,      // print
    TOKEN_TRUE,       // true
    TOKEN_FALSE,      // false
    TOKEN_IF,         // if
    TOKEN_ELIF,       // elif
    TOKEN_ELSE,       // else
    TOKEN_WHILE,      //while

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

/**
 * @brief Creates a new token.
 * @param type The type of the token.
 * @param lexeme A pointer to the start of the lexeme in the source.
 * @param length The length of the lexeme.
 * @param line The line number of the token.
 * @return A new Token struct.
 */
Token create_token(TokenType type, const char* lexeme, int length, int line);

/**
 * @brief Prints a token to stdout in a human-readable format.
 * @param token The token to print.
 */
void print_token(Token token);

/**
 * @brief Converts a TokenType enum to a string.
 * @param type The TokenType to convert.
 * @return A constant string representation (e.g., "TOKEN_PLUS").
 */
const char* token_type_to_string(TokenType type);

#endif