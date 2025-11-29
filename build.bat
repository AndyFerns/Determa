@echo off
REM Clears the console screen
cls

REM --- Config ---
SET CC=gcc
SET CFLAGS=-Wall -Wextra -g

REM --- Include Paths (Compiler Lib + Test Framework) ---
SET INCLUDES=^
    -Iinclude ^
    -Itests/include

REM --- Target Executables ---
SET COMPILER_EXE=bin\determa.exe
SET TEST_RUNNER_EXE=bin\determa_test.exe

REM --- Source Files ---
REM 1. Frontend (Lexer, Parser, TypeChecker) - In src/
SET FRONTEND_SOURCES= ^
    src\lexer.c ^
    src\token.c ^
    src\parser.c ^
    src\ast.c ^
    src\symbol.c ^
    src\typechecker.c

REM 2. Backend (VM, Bytecode, Compiler) - In src/vm/
REM Note: These files don't exist yet, but we are prepping for them!
SET BACKEND_SOURCES= ^
    src\vm\chunk.c ^
    src\vm\vm.c ^
    src\vm\compiler.c ^
    src\vm\value.c

REM Combine Lib Sources
SET LIB_SOURCES=%FRONTEND_SOURCES% %BACKEND_SOURCES%

REM Source for the main compiler executable
SET COMPILER_SOURCES=^
    src\main.c ^
    %LIB_SOURCES%

REM Source for the test runner executable
SET TEST_SOURCES=^
    tests\test_runner.c ^
    tests\test.c ^
    tests\lexer\test_lexer.c ^
    tests\parser\test_parser.c ^
    tests\typechecker\test_typechecker.c ^
    tests\vm\test_vm.c ^
    %LIB_SOURCES%

REM --- Compilation Step ---
echo Compiling Determa Compiler (%COMPILER_EXE%)...
%CC% %CFLAGS% ^
    %COMPILER_SOURCES% ^
    -o %COMPILER_EXE% ^
    -Iinclude

if %errorlevel% neq 0 (
    echo.
    echo ========================
    echo  COMPILER BUILD FAILED!
    echo ========================
    goto :eof
)
echo Compilation successful
echo.

echo Compiling Determa Test Runner (%TEST_RUNNER_EXE%)...
%CC% %CFLAGS% ^
    %TEST_SOURCES% ^
    -o %TEST_RUNNER_EXE% ^
    %INCLUDES%

if %errorlevel% neq 0 (
    echo.
    echo ===========================
    echo  TEST RUNNER BUILD FAILED!
    echo ===========================
    goto :eof
)
echo Compilation successful
echo.
echo =======================
echo  ALL BUILDS SUCCESSFUL
echo =======================
echo.
echo -------------------------------------------------------------
echo Running Determa Unit Tests...
echo -------------------------------------------------------------
echo.
%TEST_RUNNER_EXE%