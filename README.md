
# 🌲 **Determa Programming Language**  

- _A statically-typed language, custom compiler, and virtual machine — written entirely in C_

**Version:** `v0.3 “Spruce”`  
**Status:** Active Development  

Determa is a **full programming language** built completely from scratch in C — including its own **lexer, parser, type system, bytecode compiler, virtual machine, and garbage collector**.

Designed as a capstone project for **Theoretical Computer Science**, Determa merges concepts from formal languages (DFA, CFG, Scope Theory) with practical systems implementation (interpreters, memory management, call stacks, symbol tables).

---

## 🚀 **Key Features (Current Implementation)**

### 🔹 **1. Deterministic Finite Automata (Lexer)**

- High-performance tokenization  
- Zero external dependencies  
- Supports identifiers, keywords, integers, booleans, strings, operators, and punctuation

### 🔹 **2. Recursive Descent Parser (Pushdown Automaton)**

- Builds a complete **Abstract Syntax Tree (AST)**  
- Implements full statement/expression grammar  
- Supports:
  - Variable declarations  
  - Blocks  
  - If / While  
  - Function declarations & calls  
  - Return statements  
  - Binary and unary operators  

### 🔹 **3. Static Type Checker**

- Validates:
  - Type consistency in expressions  
  - Variable usage and declarations  
  - Function signatures & arity  
- Persistent Symbol Table  
- Proper lexical scope checking (blocks, functions)

### 🔹 **4. Bytecode Compiler**

- Converts AST → custom bytecode instruction set  
- Implements:
  - Global & local variable access  
  - Arithmetic & logic ops  
  - Conditional jumps  
  - Loops  
  - Function call conventions  
  - Stack-based execution model  

### 🔹 **5. Stack-Based Virtual Machine**

- Registers:
  - Operand stack  
  - Call frames  
  - Instruction pointer  
- OpCode support:
  - `OP_ADD`, `OP_SUB`, `OP_MULTIPLY`, `OP_DIVIDE`, `OP_MODULO`
  - `OP_GET_LOCAL`, `OP_SET_LOCAL`
  - `OP_GET_GLOBAL`, `OP_SET_GLOBAL`
  - `OP_CALL`, `OP_RETURN`
  - `OP_JUMP`, `OP_JUMP_IF_FALSE`, `OP_LOOP`
  - `OP_PRINT`, `OP_POP`
- Deterministic function call model  
- Correct stack unwinding + return value propagation  

### 🔹 **6. Automatic Garbage Collector**

- Mark-and-Sweep GC  
- Heaps manages:
  - Strings  
  - Functions  
  - Future object types  
- Safe during compilation (compiler roots marked)

### 🔹 **7. REPL + Script Execution**

- Interactive shell with persistent variables  
- Execute `.det` scripts directly  
- Optional parser-trace debug mode (PDA visualization)

---

## 🧩 **Project Architecture**

```plaintext

Source Code
   ↓
Lexer (DFA)
   ↓
Parser (Recursive Descent → AST)
   ↓
Type Checker (Static Analysis)
   ↓
Compiler (Bytecode Generation)
   ↓
Virtual Machine (Stack-Based Execution)
   ↓
Output
```

---

## 📦 **Quick Start**

### **Build**

```powershell
# Windows
./build.bat
```

Outputs:

- `bin/determa.exe` → language runner + REPL  
- `bin/determa_test.exe` → full unit test suite  

---

## 💻 **Usage Examples**

### **1. REPL**

```js
$ .bin\determa.exe
Determa v0.2 'Cedar'
> var x = 10;
> var y = 20;
> print x + y * 2;
Out: 50
```

### **2. Script Execution**

Executing **examples/fibonacci.det**

```python
func fib(n) : int {
    if n == 0 {
        return 0;
    } elif n == 1 {
        return 1;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

print "Fibonacci Sequence (just fib(10)):";
print fib(4);
```

Run:

```pwsh
$.bin\determa.exe examples/hello.det
```

### **3. PDA Debug Mode**

```bash
$ .bin\determa.exe --pda-debug
> 1 + 2;
PUSH: Expression
  PUSH: Term
    PUSH: Factor
```

---

## 📋 **Component Status**

| Phase | Component | Status | Details |
|------|----------|--------|---------|
| I | Lexer | ✅ Complete | DFA tokenizer |
| II | Parser | ✅ Complete | AST construction |
| III | Type Checker | ✅ Complete | Symbol tables, scopes |
| IV | Virtual Machine | ✅ Complete | Custom ISA & stack machine |
| V | Garbage Collector | ✅ Complete | Mark-and-sweep |
| VI | Control Flow | ✅ Complete | If, while, logical ops |
| VII | Functions | 🏗️ In Progress | Calls, local vars, returns |
| VIII | Closures | 🔜 Planned | Lexical captures |

---

## 🛠️ **Tech Stack**

- **Language:** C (C99)  
- **Architecture:** Hand-written compiler + VM  
- **Dependencies:** None (0 external libraries)  
- **Memory:** Custom GC  
- **Design Inspiration:** Wren, Lox, Lua, JVM bytecode  

---

## 🌿 **Philosophy**

> “Strong like the wood it's named after.”  

Determa emphasizes:

- Clarity
- Predictability  
- Theory-meets-systems design  
- Fully self-implemented infrastructure  

---

## 👨‍💻 **Author**

**Andrew Fernandes**  

Feel free to ⭐ star the repository or reach out on LinkedIn!
