/**
 * @file version.h
 * @author Andrew Fernandes
 * @brief Stores all global versioning variables accessible to the entire module
 * @version 0.1
 * @date 2025-12-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_PATCH 0
#define VERSION_NAME  "Spruce"

// Helper macro to stringify numbers if needed, 
// or just manually construct the formatted string in main.c

/**
 * @brief Helper macro to stringify numbers if needed
 * 
 * (or just manually construct the formatted string in main.c)
 */
#define VERSION_FULL "v0.3.0 'Spruce'"

#endif // VERSION_H

