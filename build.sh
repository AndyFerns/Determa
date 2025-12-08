#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# --- Config ---
CC="gcc"
CFLAGS="-Wall -Wextra -g"
INCLUDES="-Iinclude -Itests/include"

BIN_DIR="$SCRIPT_DIR/bin"
OBJ_DIR="$SCRIPT_DIR/obj"

# Ensure bin directory exists
mkdir -p "$BIN_DIR" "$OBJ_DIR"

COMPILER_EXE="bin/determa"
TEST_RUNNER_EXE="bin/determa_test"

# --- Source Files ---

# Frontend Sources
FRONTEND_SOURCES=(
    src/lexer.c
    src/token.c
    src/parser.c
    src/ast.c
    src/cli.c
    src/symbol.c
    src/typechecker.c
)

# Backend Sources
BACKEND_SOURCES=(
    src/vm/chunk.c
    src/vm/vm.c
    src/vm/compiler.c
    src/vm/value.c
    src/vm/object.c
    src/vm/memory.c
)

# Test runner sources
TEST_SOURCES=(
    tests/test_runner.c
    tests/test.c
    tests/lexer/test_lexer.c
    tests/parser/test_parser.c
    tests/parser/test_compound.c
    tests/typechecker/test_typechecker.c
    tests/vm/test_vm.c
    tests/vm/test_locals.c
    tests/vm/test_gc.c
    tests/functions/test_functions.c
)

# -----------------------------
# Build Static Library
# -----------------------------
echo "Compiling Determa Library Objects..."

OBJ_FILES=()

compile_sources() {
    for SRC in "$@"; do
        OBJ="obj/$(basename "$SRC").o"
        OBJ_FILES+=("$OBJ")
        $CC $CFLAGS -c "$SRC" -o "$OBJ" -Iinclude
    done
}

compile_sources "${FRONTEND_SOURCES[@]}"
compile_sources "${BACKEND_SOURCES[@]}"

echo "Creating static library libdeterma.a..."
ar rcs bin/libdeterma.a "${OBJ_FILES[@]}"

# -----------------------------
# Build Compiler Executable
# -----------------------------
echo "Compiling Determa Compiler ($COMPILER_EXE)..."
$CC $CFLAGS src/main.c bin/libdeterma.a -o $COMPILER_EXE -Iinclude

# -----------------------------
# Build Test Runner
# -----------------------------
echo "Compiling Determa Test Runner ($TEST_RUNNER_EXE)..."
$CC $CFLAGS "${TEST_SOURCES[@]}" bin/libdeterma.a -o $TEST_RUNNER_EXE $INCLUDES

echo "==============================="
echo "  ALL BUILDS SUCCESSFUL"
echo "==============================="
echo
echo "----------------------------------------"
echo "Running Determa Unit Tests..."
echo "----------------------------------------"
echo

./$TEST_RUNNER_EXE