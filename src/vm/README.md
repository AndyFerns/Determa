# ⚙️ Determa Virtual Machine (VM)

The Determa Virtual Machine is a **stack-based, registerless interpreter** designed to execute bytecode produced by the compiler.  
It features deterministic execution, call frames, a custom instruction set, and integrated garbage collection.

---

## 🚀 Key Features

### ✔ Stack-based execution model

- Single operand stack (`vm.stack`)
- No registers — all operations use stack values
- `push`, `pop`, and `peek` operations exposed

### ✔ Call frames

Each function call creates a `CallFrame` containing:

- Instruction pointer (`ip`)
- Pointer to local variable slots
- Reference to the active function

Supports recursion, nested calls, and argument passing.

### ✔ Custom Instruction Set (ISA)

Major opcode categories:

| Category | Instructions |
|----------|--------------|
| Constants | `OP_CONSTANT`, `OP_TRUE`, `OP_FALSE` |
| Global variables | `OP_GET_GLOBAL`, `OP_SET_GLOBAL` |
| Local variables | `OP_GET_LOCAL`, `OP_SET_LOCAL` |
| Arithmetic | `OP_ADD`, `OP_SUBTRACT`, `OP_MULTIPLY`, `OP_DIVIDE`, `OP_MODULO`, `OP_NEGATE` |
| Logic | `OP_EQUAL`, `OP_GREATER`, `OP_LESS`, `OP_NOT` |
| Control flow | `OP_JUMP`, `OP_JUMP_IF_FALSE`, `OP_LOOP` |
| Functions | `OP_CALL`, `OP_RETURN` |
| I/O | `OP_PRINT` |
| Stack mgmt | `OP_POP` |

### ✔ Function calling convention

Stack layout during calls:

```c++
[..., callee, arg1, arg2, ... argN]
```

The VM:

1. Validates arity  
2. Creates a CallFrame  
3. Sets `frame.slots` to point at the callee  
4. Executes function body  
5. On OP_RETURN, unwinds correctly and pushes the return value

### ✔ Top-level script execution

Even scripts are compiled into an implicit function.  
The final expression result is returned using `OP_RETURN`.

---

## 🧠 Memory Model

The VM cooperates with the GC by:

- Maintaining `vm.objects` linked list of all heap objects  
- Providing gray-stack pointers for incremental marking  
- Ensuring all stack values and call frames are considered roots

---

## 📂 File Overview

| File | Purpose |
|------|---------|
| `vm.c` | Main VM execution loop, stack implementation |
| `vm.h` | VM struct definitions and API |
| `opcode.h` | Instruction set architecture (ISA) |
| `value.c/h` | Value type (int, bool, object) |
| `object.c/h` | Heap objects (strings, functions) |
| `memory.c/h` | Mark-and-sweep GC implementation |
| `chunk.c/h` | Bytecode container + constant pool |

---

## 🧪 Testing

The VM is covered in:

- `tests/test_vm.c`
- `tests/test_functions.c`

Test coverage includes:

- Arithmetic  
- Stack behavior  
- Global/local variables  
- Function calls  
- Return semantics  
- Control flow  

---

## 🔮 Future Extensions

- Closures (`OP_CLOSURE`, upvalue capture)
- Native functions (C API)
- Bytecode optimizer / peephole passes

---

## ✨ Summary

The Determa VM is intentionally small, clean, and efficient — ideal for learning compiler/VM internals or extending into a more advanced language runtime.
