/**
 * @file test_lexer.h
 * @author Andrew Fernandes
 * @brief Declares the unit test functions for the Lexer
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef TEST_LEXER_H
#define TEST_LEXER_H

// A test for all simple, single-token lexemes
void test_all_tokens();

// A test to ensure line numbers are incremented correctly
void test_line_numbers();

// A test to ensure whitespace and comments are skipped
void test_whitespace_and_comments();

#endif // TEST_LEXER_H