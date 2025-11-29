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


int main(int argc, char* argv[]) {
    const char* source_string = 
        "var x = 10;\n"
        "var y = 20;\n"
        "print x + y * 2;"; //Default test string

    if (argc > 1) {
        if (strcmp(argv[1], "--pda-debug") == 0) {
            pda_debug_mode = 1;
            printf("--- PDA DEBUG MODE ENABLED ---\n");
        }
        // In the future, we'll read a file path here
    }
    
    printf("Determa Compiler [v0.1 'Balsa' Dev Build]\n");
    printf("Parsing source: \"%s\"\n\n", source_string);

    // 1. Initialize VM
    init_vm();

    // 2. Parse
    AstNode* ast = parse(source_string, pda_debug_mode);

    if (ast != NULL) {
        printf("\n--- Parse Succeeded: AST --- \n");
        print_ast(ast);

        // 3. Type Check
        if (typecheck_ast(ast)) {
            printf("Type Check Passed.\n");
            
            // 4. Compile to Bytecode
            Chunk chunk;
            init_chunk(&chunk);
            
            if (compile_ast(ast, &chunk)) {
                // 5. Run VM
                printf("\n--- Execution Output ---\n");
                interpret(&chunk);
            } else {
                printf("Compilation failed.\n");
            }
            
            free_chunk(&chunk);

        } else {
            printf("Type Check Failed.\n");
        }

        free_ast(ast);
    } else {
        printf("\n--- Parse Failed --- \n");
    }

    free_vm();
    return 0;

    // TODO: In the future, this will:
    // 1. Load a file
    // 2. Run the lexer
    // 3. Run the parser
    // 4. Run the type checker
    // 5. Compile to bytecode
    // 6. Run the VM
}
