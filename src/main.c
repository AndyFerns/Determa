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

// Simply prints a welcome message for now
#include <stdio.h>

int main(int argc, char* argv[]) {
    // Suppress unused warnings for now
    (void)argc;
    (void)argv;

    printf("Determa Compiler [v0.1 'Balsa' Dev Build]\n");
    printf("Ready to compile\n");

    // TODO: In the future, this will:
    // 1. Load a file
    // 2. Run the lexer
    // 3. Run the parser
    // 4. Run the type checker
    // 5. Compile to bytecode
    // 6. Run the VM

    return 0;
}