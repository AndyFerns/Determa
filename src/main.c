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
#include "vm/chunk.h"
#include "vm/vm.h"
#include "vm/compiler.h"
#include "colours.h"

// Version v0.2 Cedar
#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define VERSION_NAME  "Cedar"

// --- Config ---
static int pda_debug_mode = 0;

// --- UX Helpers ---

/**
 * @brief Print a standardized error message to stderr and exit.
 */
static void cli_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, B_RED "Error: " RESET);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1); // Non-zero exit code for failure
}

/**
 * @brief Print a standardized warning message.
 */
static void cli_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(YELLOW "Warning: " RESET);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

/**
 * @brief Print a success/info message.
 */
static void cli_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(CYAN "=> " RESET);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void print_version() {
    printf(B_CYAN "Determa" RESET " v%d.%d '%s'\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NAME);
    printf(GRAY "A statically-typed, garbage-collected language.\n" RESET);
}

static void print_help() {
    print_version();
    printf("\n");
    printf(BOLD "USAGE:\n" RESET);
    printf("  determa [options] [file]\n");
    printf("\n");
    printf(BOLD "OPTIONS:\n" RESET);
    printf("  " GREEN "-h, --help" RESET "        Show this help message.\n");
    printf("  " GREEN "-v, --version" RESET "     Show version information.\n");
    printf("  " GREEN "-d, --pda-debug" RESET "   Enable Parser/PDA stack trace logging.\n");
    printf("\n");
    printf(BOLD "EXAMPLES:\n" RESET);
    printf("  " cyan("determa") "                  Start Interactive REPL\n");
    printf("  " cyan("determa script.det") "       Run a script file\n");
    printf("  " cyan("determa -d script.det") "    Run with debug mode\n");
    printf("\n");
}

// --- Core Pipeline ---

static void run_source(const char* source) {
    // 1. Parse
    AstNode* ast = parse(source, config.pda_debug);

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
                 // Compiler errors are printed inside compile_ast
                 // We don't exit here to allow REPL to continue
             }
             free_chunk(&chunk);
        } 
        // TypeChecker prints its own errors
        free_ast(ast);
    } 
    // Parser prints its own errors
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
    printf(B_CYAN "Determa" RESET " v%d.%d '%s' REPL\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NAME);
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
