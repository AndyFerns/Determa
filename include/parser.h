/**
 * @file parser.h
 * @brief Defines the Parser for the Determa language
 *
 * The parser consumes tokens from the lexer and builds
 * an Abstract Syntax Tree (AST)
 */

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

/**
 * @brief The main function to parse source code
 *
 * This function initializes a parser and begins the parsing process
 *
 * @param source The source code string to parse
 * @return AstNode* The root of the generated AST (or NULL on error)
 */
AstNode* parse(const char* source);

#endif // PARSER_H