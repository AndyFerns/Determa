/**
 * @file lexer.h
 * @author Andrew Fernandes
 * @brief Defines the Lexer for the Determa language
 * 
 * the Lexer (or "scanner") walks the source code and produces a stream of tokens
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/**
 * @struct Lexer
 * @brief Holds the state of the scanner as it consumes the source code
 *
 * This struct is the hand-written implementation of the DFA
 */
typedef struct {
    const char* source;  // Pointer to the beginning of the source code string
    int start;           // Index of the start of the current lexeme
    int current;         // Index of the current character being processed
    int line;            // The current line number for error reporting
} Lexer;


/**
 * @brief Initializes a new Lexer.
 * @param source A pointer to the null-terminated source code string.
 * @return An initialized Lexer struct.
 */
Lexer init_lexer(const char* source);

/**
 * @brief Scans and returns the next token from the source code.
 *
 * This is the main "run" function of our DFA. It will be called
 * repeatedly by the parser.
 *
 * @param lexer A pointer to the Lexer.
 * @return The next Token scanned from the source.
 */
Token get_next_token(Lexer* lexer);


#endif