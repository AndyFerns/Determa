/**
 * @file test_parser.c
 * @brief Unit tests for the Determa Parser
 */

#include "test_parser.h"
#include "test.h"     // For the CHECK macro
#include "parser.h"   // The module we are testing
#include "ast.h"

/**
 * @brief A simple "hello world" test for the parser
 *
 * (STUB) This just checks that the parser function exists for now
 */
void test_parser_hello() {
    AstNode* ast = parse("1 + 2;");
    CHECK(ast == NULL, "Parser stub returns NULL");
    
    // Once implemented, we will free the AST
    // free_ast(ast);
}