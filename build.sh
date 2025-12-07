#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.

# --- Config ---
CC="gcc"
CFLAGS="-Wall -Wextra -g"
INCLUDES="-Iinclude -Itests/include"

# Ensure bin directory exists
mkdir -p bin

COMPILER_EXE="bin/determa"
TEST_RUNNER_EXE="bin/determa_test"

# --- Source Files ---

# 1. Core Library
LIB_SOURCES="src/lexer.c src/token.c src/parser.c src/ast.c src/cli.c src/symbol.c src/typechecker.c src/vm/chunk.c src/vm/vm.c src/vm/compiler.c src/vm/value.c src/vm/object.c src/vm/memory.c"

# 2. Compiler Source
COMPILER_SOURCES="src/main.c $LIB_SOURCES"

# 3. Test Source
TEST_SOURCES="tests/test_runner.c tests/test.c tests/lexer/test_lexer.c tests/parser/test_parser.c tests/parser/test_compound.c tests/typechecker/test_typechecker.c tests/vm/test_vm.c tests/vm/test_locals.c tests/vm/test_gc.c $LIB_SOURCES"

# --- Compilation Step ---

echo "Compiling Determa Compiler ($COMPILER_EXE)..."
$CC $CFLAGS $COMPILER_SOURCES -o $COMPILER_EXE -Iinclude

echo "Compiling Determa Test Runner ($TEST_RUNNER_EXE)..."
$CC $CFLAGS $TEST_SOURCES -o $TEST_RUNNER_EXE $INCLUDES

echo "======================="
echo " ALL BUILDS SUCCESSFUL"
echo "======================="
echo ""
echo "-------------------------------------------------------------"
echo "Running Determa Unit Tests..."
echo "-------------------------------------------------------------"
echo ""

./$TEST_RUNNER_EXE