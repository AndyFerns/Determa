/**
 * @file test_typechecker.h
 * @author Andrew Fernandes
 * @brief Unit test declarations for the TypeChecker module.
 *
 * This header exposes individual test routines for validating the correctness
 * of the type checker. Each test focuses on a specific behavior such as
 * variable declarations, scope rules, operator validation, and error reporting.
 *
 * @version 0.1
 * @date 2025-11-18
 */

#ifndef TEST_TYPECHECKER_H
#define TEST_TYPECHECKER_H

/**
 * @brief Tests basic variable declaration and later variable access.
 *
 * Ensures that:
 *  - A declared variable is added to the symbol table.
 *  - Accessing the variable returns a valid type.
 */
void test_tc_var_decl_and_access();

/**
 * @brief Tests referencing an undefined variable.
 *
 * Ensures the type checker:
 *  - Detects missing identifiers.
 *  - Reports a type error.
 */
void test_tc_undefined_var();

/**
 * @brief Tests redeclaration of a variable within the same scope.
 *
 * Ensures:
 *  - Duplicate declarations in the same depth produce an error.
 *  - Legitimate shadowing in outer scopes is still allowed (if implemented).
 */
void test_tc_redeclaration();

/**
 * @brief Tests type mismatch in binary operations.
 *
 * Placeholder for extended type systems.
 * Will verify:
 *  - Operations like INT + BOOL, STRING + INT, etc. are invalid.
 *  - Meaningful error reporting for mismatched operands.
 */
void test_tc_binary_op_mismatch();

#endif 