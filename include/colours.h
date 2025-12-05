/**
 * @file colors.h
 * @brief ANSI Color codes for terminal output styling.
 */

#ifndef COLORS_H
#define COLORS_H

// Reset (red)
#define RESET   "\033[0m"

// Regular Colors
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"
#define GRAY    "\033[90m"

// Bold
#define BOLD    "\033[1m"
#define B_RED   "\033[1;31m"
#define B_GREEN "\033[1;32m"
#define B_CYAN  "\033[1;36m"

// TUI Elements
#define PROMPT  B_GREEN ">> " RESET
#define ERROR   B_RED   "Error: " RESET
#define WARN    YELLOW  "Warning: " RESET
#define INFO    CYAN    "Info: " RESET

// Helper macros for simple coloring
#define cyan(str)  CYAN str RESET

#endif // COLORS_H