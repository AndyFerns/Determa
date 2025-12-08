/**
 * @file cli.c
 * @brief Implementation of the REPL and CLI-based commands
 * 
 * Isolated from main.c to maintain cleanliness and cohesion
 * 
 * @author Andrew Fernandes
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

#include "version.h"

// --- UX Helpers ---

/**
 * @brief Print a standardized error message to stderr and exit.
 */
void cli_error(const char* fmt, ...) {
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
void cli_warn(const char* fmt, ...) {
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
void cli_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(CYAN "=> " RESET);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void print_version() {
    printf(B_CYAN "Determa" RESET " v%d.%d '%s'\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NAME);
    printf(GRAY "A statically-typed, garbage-collected language.\n" RESET);
}

void print_help() {
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
