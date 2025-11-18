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
#include <string.h> 

#include "parser.h" 
#include "ast.h"   
#include "typechecker.h"
#include "vm/common.h"
#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/compiler.h"

int main(int argc, char* argv[]) {
    int pda_debug_mode = 0;
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
