/**
 * @file test.c
 * @brief Implementation of the minimal C unit testing harness.
 */

#include "test.h"

// --- Global test counters ---
int test_count = 0;
int test_pass = 0;
int test_fail = 0;

/**
 * Runs a single test function.
 */
void run_test(TestFunc func, const char* name) {
    printf("--- Running: %s ---\n", name);
    int start_fail = test_fail;
    
    // Call the test function (e.g., test_single_tokens())
    func(); 
    
    if (test_fail == start_fail) {
        printf("  [PASS] All assertions passed.\n");
    }
    printf("\n");
}

/**
 * Prints the final summary of all tests.
 */
int print_test_summary() {
    printf("===========================\n");
    printf("Test Summary:\n");
    printf("  Total Assertions: %d\n", test_count);
    printf("  Passed:           %d\n", test_pass);
    printf("  Failed:           %d\n", test_fail);
    printf("===========================\n");
    
    // Return the fail count, useful for CI/automation
    return test_fail; 
}