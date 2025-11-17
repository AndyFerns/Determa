/**
 * @file debug.h
 * @author Andrew Fernandes
 * @brief Simple Debugging Suite for the Determa Compiler
 * 
 * Provides logging macroes which can be toggled 
 * Will be used to display the PDA Stack via the C call stack
 * 
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// --- PDA Debug Tracing ---

// This global will be set by the parser
extern int pda_debug_enabled;
extern int pda_debug_indent;

/**
 * @brief Prints an indented trace message
 */
#define PDA_TRACE(message) do { \
    if (pda_debug_enabled) { \
        printf("%*s%s\n", pda_debug_indent * 2, "", message); \
    } \
} while (0)

/**
 * @brief Traces entering a parser function (PUSH)
 */
#define TRACE_ENTER(name) do { \
    if (pda_debug_enabled) { \
        printf("%*sPUSH: %s\n", pda_debug_indent * 2, "", name); \
        pda_debug_indent++; \
    } \
} while (0)

/**
 * @brief Traces exiting a parser function (POP)
 */
#define TRACE_EXIT(name) do { \
    if (pda_debug_enabled) { \
        pda_debug_indent--; \
        printf("%*sPOP:  %s\n", pda_debug_indent * 2, "", name); \
    } \
} while (0)


#endif 