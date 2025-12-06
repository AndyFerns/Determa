/**
 * @file token.c
 * @author Andrew Fernandes
 * @brief Implementation of helper functions for the Token struct.
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "token.h"
#include <string.h>

/**
 * Creates a new token
 */
Token create_token(TokenType type, const char* lexeme, int length, int line) {
    Token token;

    token.type = type;
    token.lexeme = lexeme;
    token.length = length;
    token.line = line;

    return token;
}

/**
 * @brief Prints a token to stdout in a human-readable format
 * 
 * @param token The token to print
 */
void print_token(Token token) {
    printf("[Line %-3d] %-15s '%.*s'\n",
           token.line,
           token_type_to_string(token.type),
           token.length,
           token.lexeme);
}


/**
 * Converts a TokenType enum to a string.
 */
const char* token_type_to_string(TokenType type) {
    switch (type) {
        // Single-Character tokens
        case TOKEN_LPAREN:    return "TOKEN_LPAREN";
        case TOKEN_RPAREN:    return "TOKEN_RPAREN";
        case TOKEN_LEFT_BRACE:  return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "TOKEN_RIGHT_BRACE"; 
        case TOKEN_COMMA:       return "TOKEN_COMMA";
        case TOKEN_COLON:       return "TOKEN_COLON";

        // Arithmetic Operators and Assignment
        case TOKEN_PLUS:      return "TOKEN_PLUS";
        case TOKEN_MINUS:     return "TOKEN_MINUS";
        case TOKEN_STAR:      return "TOKEN_STAR";
        case TOKEN_SLASH:     return "TOKEN_SLASH";
        case TOKEN_PERCENT:   return "TOKEN_PERCENT";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_EQUALS:    return "TOKEN_EQUALS";

        case TOKEN_PLUS_EQUAL:    return "TOKEN_PLUS_EQUAL";    
        case TOKEN_MINUS_EQUAL:   return "TOKEN_MINUS_EQUAL";   
        case TOKEN_STAR_EQUAL:    return "TOKEN_STAR_EQUAL";    
        case TOKEN_SLASH_EQUAL:   return "TOKEN_SLASH_EQUAL";   
        case TOKEN_PERCENT_EQUAL: return "TOKEN_PERCENT_EQUAL"; 

        case TOKEN_BANG:          return "TOKEN_BANG";
        case TOKEN_BANG_EQUAL:    return "TOKEN_BANG_EQUAL";
        case TOKEN_EQUAL_EQUAL:   return "TOKEN_EQUAL_EQUAL";
        case TOKEN_GREATER:       return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LESS:          return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:    return "TOKEN_LESS_EQUAL";
        case TOKEN_TRUE:          return "TOKEN_TRUE";
        case TOKEN_FALSE:         return "TOKEN_FALSE";

        // Literals
        case TOKEN_INT:       return "TOKEN_INT";
        case TOKEN_ID:        return "TOKEN_ID";
        case TOKEN_STRING:    return "TOKEN_STRING";

        // Keywords
        case TOKEN_VAR:       return "TOKEN_VAR";
        case TOKEN_PRINT:     return "TOKEN_PRINT";
        case TOKEN_IF:          return "TOKEN_IF";    
        case TOKEN_ELIF:        return "TOKEN_ELIF";  
        case TOKEN_ELSE:        return "TOKEN_ELSE";  
        case TOKEN_WHILE:       return "TOKEN_WHILE"; 
        case TOKEN_FUNC:        return "TOKEN_FUNC";
        case TOKEN_RETURN:      return "TOKEN_RETURN";
        case TOKEN_TYPE_INT:    return "TOKEN_TYPE_INT";
        case TOKEN_TYPE_BOOL:   return "TOKEN_TYPE_BOOL";
        case TOKEN_TYPE_STRING: return "TOKEN_TYPE_STRING";
        case TOKEN_TYPE_VOID:   return "TOKEN_TYPE_VOID";

        // Control Tokens
        case TOKEN_ERROR:     return "TOKEN_ERROR";
        case TOKEN_EOF:       return "TOKEN_EOF";
        
        default:              return "TOKEN_UNKNOWN";
    }
}