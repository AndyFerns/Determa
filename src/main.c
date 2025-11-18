/**
 * @file main.c
 * @author Andrew Fernandes
 * @brief Main entry point for the Determa compiler
 * @version v0.1 'Balsa' Dev Build
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <string.h> // For strcmp

#include "parser.h" 
#include "ast.h"
#include "typechecker.h"

int main(int argc, char* argv[]) {
    int pda_debug_mode = 0;
    const char* source_string = 
        "var x = 10;\n"
        "var y = 20;\n"
        "print x + y * 2;";; // Default test string

    if (argc > 1) {
        if (strcmp(argv[1], "--pda-debug") == 0) {
            pda_debug_mode = 1;
            printf("--- PDA DEBUG MODE ENABLED ---\n");
        }
        // In the future, we'll read a file path here
    }
    
    printf("Determa Compiler [v0.1 'Balsa' Dev Build]\n");
    printf("Parsing source: \"%s\"\n\n", source_string);

    // --- NEW: Run the parser ---
    AstNode* ast = parse(source_string, pda_debug_mode);

    if (ast != NULL) {
        printf("\n--- Parse Succeeded: AST --- \n");
        print_ast(ast);

        // 2. Type Check (NEW)
        printf("\n--- Running Type Checker --- \n");
        if (typecheck_ast(ast)) {
             printf("Type Check Passed.\n");
             // Phase 4: Generate Code / Run VM would go here
        } else {
             printf("Type Check Failed.\n");
        }

        free_ast(ast);
    } else {
        printf("\n--- Parse Failed --- \n");
    }

    return 0;

    // TODO: In the future, this will:
    // 1. Load a file
    // 2. Run the lexer
    // 3. Run the parser
    // 4. Run the type checker
    // 5. Compile to bytecode
    // 6. Run the VM
}