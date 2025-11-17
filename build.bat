@echo off
REM Clears the console screen
cls

REM Define compiler and flags
SET CC=gcc
SET CFLAGS=-Wall -Wextra -g

REM --- We now have TWO include paths ---
REM -Iinclude (for compiler code like lexer.h)
REM -Itests/include (for test code like test.h)
SET INCLUDES=-Iinclude -Itests/include

REM Define the target executable name
SET TARGET=bin\determa.exe

REM --- All sources required to build the TEST RUNNER ---
SET SOURCES=src\lexer.c ^
    src\token.c ^
    tests\test.c ^
    tests\lexer\test_lexer.c ^
    tests\test_runner.c


REM --- Compilation Step ---
echo Compiling Determa test runner...
%CC% %CFLAGS% %SOURCES% -o %TARGET% %INCLUDES%

REM --- Check if compilation was successful and run ---
if %errorlevel% neq 0 (
    echo.
    echo =====================
    echo  COMPILATION FAILED!
    echo =====================
    goto :eof
)

echo.
echo =======================
echo  COMPILATION SUCCESSFUL
echo =======================
echo.
echo -------------------------------------------------------------
echo Running Determa Unit Tests...
echo -------------------------------------------------------------
echo.
%TARGET%