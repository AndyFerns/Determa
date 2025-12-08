# 🌳 Roadmap to Determa v1.0 'Oak'

- Current Status: v0.2 'Cedar' (Turing Complete Scripting)
- Target: v1.0 'Oak' (Structured, Modular Programming Language)

To achieve the "Complete Language" milestone for a v1.0 release, Determa must evolve from a linear execution engine into a structured one. The 'Oak' release focuses on Functions, Local Scope, and Native Interop.

## Phase 8: The Stack Architecture (Local Variables)

Currently, all variables are global. This limits recursion and modularity.

- [x] Compiler Upgrade:

- [x] Implement ScopeDepth tracking in the compiler.

- [x] Modify variable resolution: Search for locals (stack slots) before globals.

### VM Upgrade

- [x] Implement OP_GET_LOCAL and OP_SET_LOCAL.

- [x] Unlike globals (lookup by index), locals are accessed by Stack Offset (e.g., "value at slot 3").

- [x] Implement OP_POP logic to clean up the stack when exiting a { block }.

## Phase 9: First-Class Functions

- The defining feature of modern programming. Allows code reuse and recursion.

- [x] Function Syntax:

- [x] Add func keyword.

- [x] Parser support: fun add(a, b) { return a + b; }.

## [ ] Function Objects

- [x] Create ObjFunction struct.

- [x] Functions should be "First Class" (can be stored in variables).

- [x] Call Stack:

- [x] Implement CallFrame struct in the VM.

- [x] Implement OP_CALL and OP_RETURN.

- [x] Logic: Save the current instruction pointer (ip), jump to the function, run it, and restore ip on return.

## Phase 10: Native Functions (The Standard Library)

- Bridging Determa with the real world (C).

- [ ] Native Object:

- [ ] Create ObjNative to wrap C function pointers.

- [ ] Standard Library:

- [ ] clock(): Benchmark code performance.

- [ ] input(): Read user input from the terminal.

- [ ] str(x): Convert numbers to strings.

- [ ] len(arr): Get lengths.

## Phase 11: Data Structures (Arrays/Lists)

- Moving beyond simple scalars.

- [ ] Syntax:

- [ ] Parse list literals: var list = [1, 2, 3];.

- [ ] Parse indexing: print list[0];.

- [ ] Implementation:

- [ ] Create ObjList (Dynamic array of Values).

- [ ] Integrate with Garbage Collector (Marking a list must mark its contents).

## Phase 12: The Polish (Pre-Release)

- Quality of Life improvements.

- [ ] Stack Traces: When an error occurs inside a function, print the call stack (Line numbers and function names).

- [ ] File Import: import "math.det"; to split code across files.

- [ ] Optimization: Constant Folding (e.g., compile 1 + 2 directly as 3).

## 🎯 The Final Demo (LinkedIn Showcase)

Once v1.0 'Oak' is complete, the language will be capable of running complex algorithms. The showcase script will be Recursive Fibonacci or Quicksort, demonstrating:

- Recursion (Stack frames).

- Local scoping.

- Performance (using clock()).

```rust
// v1.0 Goal Syntax
fun fib(n) {
    if n < 2 { return n; }
    return fib(n - 1) + fib(n - 2);
}

var start = clock();
print fib(30);
print "Time taken: " + (clock() - start);
```
