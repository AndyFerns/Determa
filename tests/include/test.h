/**
 * @file test.h
 * @author Andrew Fernandes
 * @brief A minimal C unit testing suite
 * 
 * Provides a CHECK() macro for assertions and functions
 * to run tests and print a summary.
 * 
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>

// --- Global test counters ---
// These are defined in test.c
extern int test_count;
extern int test_pass;
extern int test_fail;

/**
 * @def CHECK(expr, msg)
 * @brief The main assertion macro.
 *
 * Checks if 'expr' is true. If not, it prints the fail
 * message 'msg' along with file/line info and increments
 * the fail counter.
 */
#define CHECK(expr, msg) do { \
    test_count++; \
    if (!(expr)) { \
        printf("  [FAIL] %s \n\t\t(Assertion failed: %s, %s:%d)\n", msg, #expr, __FILE__, __LINE__); \
        test_fail++; \
    } else { \
        test_pass++; \
    } \
} while (0)

/**
 * @brief A function pointer type for a test case.
 * A test case is just a function that takes no arguments
 * and returns nothing.
 */
typedef void (*TestFunc)();

/**
 * @brief Runs a single test function.
 * @param func The test function to run.
 * @param name A name for the test, for printing.
 */
void run_test(TestFunc func, const char* name);

/**
 * @brief Prints the final summary of all tests.
 * @return The total number of failed tests (0 for success).
 */
int print_test_summary();

#endif 