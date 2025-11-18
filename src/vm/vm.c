#include "vm/vm.h"
#include <stdio.h>

void init_vm() {
    // TODO: Initialize stack
}

void free_vm() {
    // TODO: Cleanup
}

InterpretResult interpret(Chunk* chunk) {
    printf("VM: Interpret called (Stub)\n");
    // TODO: Implement fetch-decode-execute loop
    return INTERPRET_OK;
}