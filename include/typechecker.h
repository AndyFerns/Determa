/**
 * @file typechecker.h
 * @author Andrew Fernandes
 * @brief Public API for the semantic analyzer. 
 * 
 * Semantic Analysis for Determa
 * Walks the AST to verify types and variable usage.
 * @version 0.1
 * @date 2025-11-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"

/**
 * @brief Runs the semantic analysis on the AST.
 * @param root The root of the AST.
 * @return 1 if the program is semantically valid, 0 if there are errors.
 */
int typecheck_ast(AstNode* root);

#endif // TYPECHECKER_H