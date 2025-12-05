/**
 * @file test_compound.h
 * @brief Unit tests for Modulo and Compound Assignment features.
 */

#ifndef TEST_COMPOUND_H
#define TEST_COMPOUND_H

// Tests if '%' is parsed as a binary operator with correct precedence
void test_parser_modulo();

// Tests if 'x += 1' is correctly desugared into 'x = x + 1'
void test_parser_compound_assignment();

#endif // TEST_COMPOUND_H