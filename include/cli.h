/**
 * @file cli.h
 * @author Andrew Fernandes
 * @brief Headerfile for isolated CLI operations
 * 
 * TODO: add repl.h/c and isolate from main once build is stable
 * 
 * @version 0.1
 * @date 2025-12-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */


/**
 * @brief Print a standardized error message to stderr and exit.
 */
void cli_error(const char* fmt, ...);

/**
 * @brief Print a standardized warning message.
 */
void cli_warn(const char* fmt, ...);


/**
 * @brief Print a success/info message.
 */
void cli_info(const char* fmt, ...);


/**
 * @brief Simple helper function to print the current version
 * 
 * Versioning changes are reflected 
 * 
 */
void print_version();

void print_help();