/**
 * @file main.c
 * @author Andrew Fernandes
 * @brief Main entry point for the Determa compiler
 * @version v0.2 'Cedar' Dev Build
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "parser.h" 
#include "ast.h"   
#include "typechecker.h"
#include "vm/common.h"
#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/compiler.h"

// --- Config ---
static int pda_debug_mode = 0;


/**
 * @brief Executes a source string through the entire pipeline.
 * Pipeline: Lex -> Parse -> TypeCheck -> Compile -> Run
 */
static void run(const char* source) {
        // 1. Parse
    AstNode* ast = parse(source, pda_debug_mode);

    if (ast != NULL) {
        // 2. Type Check
        if (typecheck_ast(ast)) {
             // 3. Compile
             Chunk chunk;
             init_chunk(&chunk);
             
             if (compile_ast(ast, &chunk)) {
                 // 4. Run
                 interpret(&chunk);
             } else {
                 // Compile error
             }
             free_chunk(&chunk);
        } 
        // TypeCheck errors are printed inside the module
        free_ast(ast);
    } 
    // Parse errors are printed inside the module
}

/**
 * @brief Method to read the contents of a file containing .det code
 * 
 * @param path pointer to the path where the file is located
 * @return char* 
 */
static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

/**
 * @brief Runs a script file.
 */
static void runFile(const char* path) {
    // --- Check file extension ---
    const char* ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".det") != 0) {
        fprintf(stderr, "Warning: File '%s' does not have a .det extension.\n", path);
        // We warn but proceed, or exit(64) if you want to be strict
    }
    char* source = readFile(path);
    
    // Initialize Persistent Systems
    init_vm();
    init_typechecker();
    init_compiler();

    run(source);
    
    free(source);
    free_typechecker();
    free_vm();
}


/**
 * @brief Starts the Interactive REPL (Read-Eval-Print Loop).
 */
static void repl() {
    printf("Determa v0.2 'Cedar' REPL\n");
    printf("Type 'exit' to quit.\n");

    // Initialize Persistent Systems (Variables stay alive)
    init_vm();
    init_typechecker();
    init_compiler();

    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "exit") == 0) break;

        run(line);
    }

    free_typechecker();
    free_vm();
}


int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "--pda-debug") == 0) {
        pda_debug_mode = 1;
        printf("--- PDA DEBUG MODE ENABLED ---\n");
        // Shift args so we can handle files correctly
        argv++;
        argc--;
    }

    printf("Determa Compiler [v0.2 'Cedar' Dev Build]\n");
    // printf("Parsing source: \"%s\"\n\n", source_string);

    // no arguments, defaults to REPL mode
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        // adding a file path as an argument
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: determa [path] [--pda-debug]\n");
        exit(64);
    }

    return 0;
}
