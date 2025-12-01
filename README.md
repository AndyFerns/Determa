# 🌲 Determa Programming Language

**Version**: v0.2 'Cedar' Dev Build

**Status**: Active Development (Phase 5 Complete)

Determa is a statically-typed, garbage-collected programming language built from scratch in C. It runs on a custom high-performance stack-based Virtual Machine.

Designed as a capstone project for Theoretical Computer Science, Determa bridges the gap between abstract theory (DFAs, CFGs, Type Theory) and practical systems engineering (Bytecode, Virtualization, Memory Management).

## Architecture

Determa implements a complete compiler pipeline:

1. Lexer: Hand-optimized Deterministic Finite Automata (DFA) for tokenization.

2. Parser: Recursive Descent Parser (Pushdown Automaton) creating an Abstract Syntax Tree (AST).

3. Semantic Analysis: Static Type Checker with persistent Symbol Tables for scope validation.

4. Compiler: Single-pass bytecode generation.

5. Virtual Machine: Stack-based VM with a custom instruction set (ISA).

6. Memory Manager: Mark-and-Sweep Garbage Collector for automatic memory management.

## Quick Start

**Prerequisites**

- GCC Compiler (MinGW for Windows, or standard GCC for Linux/Mac).

- A terminal (PowerShell, CMD, or Bash).

**Building the Project**

The project includes a unified build script.

```bash
# Windows
.\build.bat
```

This generates two executables in the bin/ directory:

- determa.exe: The main compiler and REPL.

- determa_test.exe: The internal unit test suite.

## Usage

1. **Interactive REPL**

Launch the executable without arguments to enter the Read-Eval-Print Loop. The REPL supports persistent variables across lines.

```python
$ .\bin\determa.exe
Determa v0.2 'Cedar' REPL
Type 'exit' to quit.
> var x = 10;
> var y = 20;
> print x + y * 2;
Out: 50
```

2. **File Execution**

Pass a .det file path to compile and run a script.

Example Script (examples/hello.det):

```java
var greeting = "Hello ";
var target = "World";
print greeting + target;
```

Run:

```pwsh
$ .\bin\determa.exe examples/hello.det
```

3. **Debug Mode (PDA Trace)**

Use the --pda-debug flag to visualize the Parser's recursive descent logic (Pushdown Automaton stack trace).

```bash
$ .\bin\determa.exe --pda-debug
> 1 + 2;
PUSH: Declaration
  PUSH: Statement
    PUSH: Expression
      ...
```

## Implementation Status

Phase

Component

Status

Description

I

Lexer

✅ Done

DFA-based tokenization.

II

Parser

✅ Done

AST construction & Grammar definition.

III

Type Checker

✅ Done

Type inference & Scope validation.

IV

Virtual Machine

✅ Done

Bytecode execution & Stack manipulation.

V

Garbage Collector

✅ Done

Mark-and-Sweep GC & String support.

VI

Control Flow

### Next

Booleans, if, while, and Logic.

## Tech Stack

- Language: C (C99 Standard)

- Build System: Custom Batch Script

- Dependencies: None (Zero-dependency implementation)

"Strong like the wood it's named after."

## Author

Andrew Fernandes
