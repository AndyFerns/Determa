@echo off
REM Clears the console screen
cls

REM Define compiler and flags
SET CC=gcc
SET CFLAGS=-Wall -Wextra -g
SET INCLUDES=-Iinclude

REM Define the target executable name
SET TARGET=bin\determa.exe

REM --- Updated sources for the COMPILER ---
SET SOURCES=main.c src\lexer.c src\token.c

REM --- Compilation Step ---
echo Compiling Determa compiler...
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
echo Running Determa (Lexer Test)...
echo -------------------------------------------------------------
echo.
%TARGET%