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
#include <stdarg.h>

#include "parser.h" 
#include "ast.h"   
#include "typechecker.h"
#include "vm/common.h"
#include "vm/vm.h"
#include "vm/compiler.h"
#include "colours.h"
#include "cli.h"

#include "version.h"

// --- Config ---

typedef struct {
    int pda_debug;
    int show_version;
    int show_help;
    const char* file_path;
} CliConfig;

static CliConfig config = {0, 0, 0, NULL};

// --- Core Pipeline ---

static void run_source(const char* source) {
 // 1. Parse
    AstNode* ast = parse(source, config.pda_debug);

    if (ast == NULL) {
        // Parser already printed errors
        return;
    }

    // 2. Type Check
    if (!typecheck_ast(ast)) {
        // Typechecker prints its own errors
        free_ast(ast);
        return;
    }

    // 3. Compile AST into a function object
    ObjFunction* function = compile_ast(ast);  // <-- new API

    if (function == NULL) {
        // Compiler printed its own errors
        free_ast(ast);
        return;
    }

    // 4. Run on the VM
    interpret(function);

    // 5. Cleanup AST (the function is now owned by the VM/GC)
    free_ast(ast);
}

static char* read_file_contents(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        cli_error("Could not open file \"%s\". Check permissions or path.", path);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        cli_error("Not enough memory to read \"%s\".", path);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        cli_error("Could not read file \"%s\".", path);
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

static void run_file_mode() {
    // File extension check (Polished)
    const char* ext = strrchr(config.file_path, '.');
    if (!ext || strcmp(ext, ".det") != 0) {
        cli_warn("File '%s' does not end with .det extension.", config.file_path);
    }

    // Give feedback to the user
    cli_info("Reading file: %s", config.file_path);

    char* source = read_file_contents(config.file_path);
    
    // Initialize Persistent Systems
    init_vm();
    init_typechecker();
    init_compiler();

    run_source(source);
    
    free(source);
    free_typechecker();
    free_vm();
}

static void print_repl_help() {
    printf("\n" BOLD "REPL Commands:" RESET "\n");
    printf("  " GREEN "exit" RESET "    Quit the REPL.\n");
    printf("  " GREEN "clear" RESET "   Clear the screen.\n");
    printf("  " GREEN "help" RESET "    Show this menu.\n");
    printf("\n");
}

static void run_repl_mode() {
    printf(B_CYAN "Determa" RESET " v%d.%d.%d '%s' REPL\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_NAME);
    printf(GRAY "Type 'help' for commands, 'exit' to quit.\n" RESET);
    printf(GRAY "---------------------------------------\n" RESET);

    init_vm();
    init_typechecker();
    init_compiler();

    char line[1024];
    for (;;) {
        printf(PROMPT);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        
        // Strip newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        // REPL Commands
        if (strcmp(line, "exit") == 0) break;
        if (strcmp(line, "help") == 0) {
            print_repl_help();
            continue;
        }
        if (strcmp(line, "clear") == 0) {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            continue;
        }
        
        // Handle empty lines
        if (strlen(line) == 0) continue;

        run_source(line);
    }

    free_typechecker();
    free_vm();
}


int main(int argc, char* argv[]) {
    // Argument Parsing
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            config.show_help = 1;
        } 
        else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
            config.show_version = 1;
        } 
        else if (strcmp(arg, "--pda-debug") == 0 || strcmp(arg, "-d") == 0) {
            config.pda_debug = 1;
        } 
        else {
            // It must be a file path
            if (config.file_path == NULL) {
                config.file_path = arg;
            } else {
                cli_error("Unexpected argument '%s'. Only one file supported.", arg);
            }
        }
    }

    // Execute respective mode
    if (config.show_help) {
        print_help();
        return 0;
    }
    if (config.show_version) {
        print_version();
        return 0;
    }

    if (config.file_path != NULL) {
        run_file_mode();
    } else {
        run_repl_mode();
    }

    return 0;
}
