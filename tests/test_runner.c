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
#include "test_typechecker.h"
#include "test_vm.h"
#include "typechecker.h" // For init
#include "vm/compiler.h" // For init

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

    // Phase 2 (Parser) Test Suite
    printf("\n"); // separator
    run_test(test_parser_integer_literal, "Parser - Integer Literal");
    run_test(test_parser_simple_binary_op, "Parser - Simple Binary Op");
    run_test(test_parser_operator_precedence, "Parser - Operator Precedence");
    run_test(test_pda_debug_output, "Parser - PDA Debug Output");
    run_test(test_parser_var_declaration, "Parser - Var Declaration");
    run_test(test_parser_print_statement, "Parser - Print Statement");
    run_test(test_parser_program, "Parser - Full Program");


    // Reset before TC tests to ensure clean state
    init_typechecker(); 
    // Phase 3 (Typechecker) Test Suite
    printf("\n");
    run_test(test_tc_var_decl_and_access, "TypeChecker - Valid Var Decl & Access");
    run_test(test_tc_undefined_var, "TypeChecker - Undefined Variable Error");
    run_test(test_tc_redeclaration, "TypeChecker - Redeclaration Error");

    // Phase 4 Tests (VM)
    printf("\n");
    test_vm_suite();

    // --- Add more tests here as we build new modules ---
    // run_test(test_parser_simple, "Parser - Simple Expression");

    // --- Print the final summary ---
    int failures = print_test_summary();

    // Return 0 if all tests passed, 1 if any failed
    return failures;
}