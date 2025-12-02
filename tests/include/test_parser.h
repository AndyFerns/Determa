/**
 * @file test_parser.h
 * @brief Declares the unit test functions for the Parser
 */

#ifndef TEST_PARSER_H
#define TEST_PARSER_H

/**
 * @brief Tests parsing a simple integer literal
 */
void test_parser_integer_literal();

/**
 * @brief Tests parsing a simple binary operation (1 + 2)
 */
void test_parser_simple_binary_op();

/**
 * @brief Tests operator precedence (1 + 2 * 3)
 */
void test_parser_operator_precedence();

/**
 * @brief Tests the PDA debug output
 */
void test_pda_debug_output();

/**
 * @brief Test parser variable declaration
 */
void test_parser_var_declaration(); 

/**
 * @brief Test parser print statement
 */
void test_parser_print_statement(); 

/**
 * @brief Test parser program formation + parsing
 */
void test_parser_program();

// --- (Phase 6) ---
void test_parser_bool_literals();
void test_parser_logic_precedence();

// --- Phase 7 testtcases ---
void test_parser_block();
void test_parser_if();
void test_parser_while();

#endif // TEST_PARSER_H