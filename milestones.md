# 🗺️ Determa Language & Compiler Roadmap

This document outlines the milestones to build the Determa programming language, a simple C-like language. This project builds on a completed Regex/DFA engine to create a full compiler-interpreter.

The project is structured in phases. Phases 2 and 4 are specifically designed to align with advanced TCS topics for practical exam study.

## Phase 1: The Lexer (Lexical Analysis)

Goal: Convert raw source code text into a stream of discrete tokens.
Status: 90% Complete (The core engine is done).

- Syllabus Link: Modules 1 & 2 (Finite Automata, Regular Languages)

- [x] Core Engine: Build a Regex Engine (NFA/DFA).

- [ ] Define Tokens: Create an enum in include/tokens.h for all of Determa's tokens (e.g., TOKEN_VAR, TOKEN_ID, TOKEN_INT, TOKEN_PLUS, TOKEN_LPAREN, TOKEN_SEMICOLON, TOKEN_EOF).

- [ ] Token Struct: Create a Token struct to hold the token's type, its string value (lexeme), and its line number.

- [ ] Implement the Lexer: Write the main lexer.c module.

- [ ] Create a Lexer struct that holds the source code and current position.

- [ ] Write a get_next_token() function. This function will use your DFA engine to match the longest possible token from the current position.

- [ ] Crucial: The Lexer must handle whitespace and comments by skipping them.

## Phase 2: The Parser (Syntactic Analysis)

Goal: Convert the stream of tokens into a structured "Abstract Syntax Tree" (AST).
Status: To Be Started. (This is the most critical exam-study phase).
Syllabus Link: Modules 3 & 4 (CFGs, PDAs)

- [ ] Define the Determa CFG: Formally write down the Context-Free Grammar for the language.

// Example Determa Grammar

```bash
Program        -> StatementList
StatementList  -> Statement (Statement)*
Statement      -> VarDecl | Assignment | PrintStmt
VarDecl        -> "var" ID "=" Expression ";"
Assignment     -> ID "=" Expression ";"
PrintStmt      -> "print" Expression ";"
Expression     -> Term ( ("+"|"-") Term )*
Term           -> Factor ( ("*"|"/") Factor )*
Factor         -> Primary | Unary
Unary          -> "-" Primary
Primary        -> INT | ID | "(" Expression ")"
```

- [ ] AST Node Structs: In include/ast.h, define structs for each node in the AST (e.g., struct BinaryOpNode, struct VarDeclNode, struct NumberNode). These structs are the in-memory representation of your Parse Tree.

- [ ] Implement Recursive Descent Parser: This is the core of the parser.

- [ ] Create parser.c.

- [ ] Write a function for each grammar rule (e.g., parse_statement(), parse_expression()).

- [ ] These functions will consume tokens from the Lexer and recursively build the AST.

- [ ] [EXAM FEATURE] Implement "PDA Debug Mode":

- [ ] Add a --pda-debug flag.

- [ ] In your parser functions, print PUSH(RuleName) on entry and POP(RuleName) on exit.

- [ ] Result: This visually demonstrates your parser acting as a Pushdown Automaton (PDA), using the C call stack as its stack. (Fulfills LO 4.4)

## Phase 3: The Interpreter (Execution)

Goal: "Run" the Abstract Syntax Tree to get a result.
Status: To Be Started. (This is where the logic happens).

### Syllabus Link: Module 6 (Undecidability)

- [ ] Implement the Symbol Table:

- [ ] Create symbol_table.c.

- [ ] This will be a simple Hash Map or Linked List that stores variable names and their values (e.g., {"x": 10}).

- [ ] Implement the "Visitor" / Evaluator:

- [ ] Create evaluator.c.

- [ ] Write a recursive function

```c  
evaluate(Node* node, SymbolTable* table).
```

- [ ] This function will walk the AST and perform the actions.

BinaryOpNode: Evaluate left, evaluate right, perform the math.

VarDeclNode: Evaluate the expression, store the result in the SymbolTable.

IDNode: Look up the variable's value in the SymbolTable.

- [ ] [EXAM FEATURE] Implement "The Halting Guard":

- [ ] Add an instruction counter to your evaluate loop.

- [ ] If the counter exceeds a large number (e.g., 1,000,000), stop execution and report a "Potential Infinite Loop" error.

- [ ] Result: This demonstrates your understanding of the Halting Problem (LO 6.4)—that you cannot statically prove a loop will halt, so you must use a dynamic guard.

## Phase 4: Advanced Features (The "Wow" Factor)

Goal: Integrate advanced TCS theory to make this a truly unique project.
Status: To Be Started. (This is the resume polish).

### Syllabus Link: Module 5 (Turing Machines)

- [ ] [EXAM FEATURE] Implement a Universal Turing Machine (UTM) Module:

- [ ] Add a new built-in function to Determa's parser and evaluator: run_tm(...).

- [ ] The syntax in Determa might be: var result = run_tm(tape_string, rules_string, start_state);

- [ ] Write the C function native_run_tm(...) that is called by your evaluator.

- [ ] This C function will be a full simulator for a simple Turing Machine.

- [ ] Result: You will have implemented a Universal Turing Machine (a program that can simulate any other TM), directly satisfying LO 5.3.

- [ ] Error Reporting: Improve the Lexer and Parser to report errors with line numbers (e.g., Syntax Error: Expected ')' on line 5).

## Phase 5: The "Compiler" Upgrade (Stretch Goal)

Goal: Convert Determa code into another, lower-level language instead of running it.

- [ ] Define a Simple Stack-Based IR: Create a simple instruction set (e.g., PUSH 5, PUSH 10, ADD, STORE "x", LOAD "x").

- [ ] Write a Code Generator: Write a new recursive function generate_code(Node* node) that walks the AST and prints the IR instructions instead of evaluating them.

- [ ] Write a Virtual Machine (VM): A separate C program that can read and execute your new IR. This VM is the target for your compiler.
