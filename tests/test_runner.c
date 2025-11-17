/**
 * @file test_runner.c
 * @brief Main entry point for the Determa test suite.
 *
 * This file contains the main function to drive the test harness.
 */

#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "test_lexer.h"
#include "test_parser.h"

int main(int argc, char* argv[]) {
    // Suppress unused warnings
    (void)argc; 
    (void)argv;

    printf("Starting Determa Test Suite...\n\n");
    
    // --- Run all our test suites ---

    // Phase 1 (lexer) Test suite
    run_test(test_all_tokens, "Lexer - Comprehensive Token Stream");
    run_test(test_line_numbers, "Lexer - Line Number Increments");
    run_test(test_whitespace_and_comments, "Lexer - Whitespace and Comment Skipping");

    // --- NEW TEST ---
    printf("\n"); // Add a separator
    run_test(test_parser_integer_literal, "Parser - Parsing a simple integer literal");

    // --- Add more tests here as you build new modules ---
    // run_test(test_parser_simple, "Parser - Simple Expression");

    // --- Print the final summary ---
    int failures = print_test_summary();

    // Return 0 if all tests passed, 1 if any failed
    return failures;
}