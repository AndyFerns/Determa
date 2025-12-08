/**
 * @file test_functions.c
 * @brief High-level function tests for Determa (calls, scope, recursion).
 */

#include <string.h>
#include <stdio.h>

#include "test_functions.h"
#include "test.h"

#include "parser.h"
#include "typechecker.h"
#include "vm/compiler.h"
#include "vm/vm.h"
#include "vm/value.h"
#include "ast.h"


/* -------------------------------------------------------------
 * Helper: compile and run, get return value from final expression
 * 
 * In Determa, the last expression statement in a script leaves
 * its value on the stack, which we can then inspect.
 * ------------------------------------------------------------- */
static Value run_and_get_result(const char* source) {
    AstNode* ast = parse(source, 0);
    CHECK(ast != NULL, "Parse must succeed");

    CHECK(typecheck_ast(ast), "Typecheck must succeed");

    ObjFunction* fn = compile_ast(ast);
    CHECK(fn != NULL, "Compilation must succeed");

    init_vm();
    InterpretResult result = interpret(fn);
    CHECK(result == INTERPRET_OK, "Execution must succeed");

    // The final expression value should be on stack
    CHECK(vm.stackTop > vm.stack, "Result value must exist on stack");
    Value v = vm.stack[0];

    free_vm();
    free_ast(ast);
    return v;
}


/* -------------------------------------------------------------
 * TEST 1: Simple function call - add(a,b)
 * ------------------------------------------------------------- */
static void test_add_function() {
    Value v = run_and_get_result(
        "func add(a, b): int { "
        "    return a + b; "
        "} "
        "add(400, 700);"  // Expression statement - result stays on stack
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 1100, "add(400,700) must return 1100");
}


/* -------------------------------------------------------------
 * TEST 2: Function composition - twice & quad
 * ------------------------------------------------------------- */
static void test_twice_quad() {
    Value v = run_and_get_result(
        "func twice(x): int { "
        "    return x * 2; "
        "} "
        "func quad(x): int { "
        "    return twice(twice(x)); "
        "} "
        "quad(5);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 20, "quad(5) must return 20");
}


/* -------------------------------------------------------------
 * TEST 3: Recursive Fibonacci
 * ------------------------------------------------------------- */
static void test_recursion_fib() {
    Value v = run_and_get_result(
        "func fib(n): int { "
        "    if (n <= 1) { "
        "        return n; "
        "    } "
        "    return fib(n - 1) + fib(n - 2); "
        "} "
        "fib(8);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 21, "fib(8) must return 21");
}


/* -------------------------------------------------------------
 * TEST 4: Variable scoping with blocks
 * ------------------------------------------------------------- */
static void test_scoping() {
    // Test that inner scope shadows outer, then outer is restored
    Value v = run_and_get_result(
        "var a = 10; "
        "{ "
        "    var a = 99; "
        "} "
        "a;"  // Final expression - should be outer 'a'
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 10, "Outer 'a' must be restored after block");
}


/* -------------------------------------------------------------
 * TEST 5: Nested functions and lexical scope
 * ------------------------------------------------------------- */
static void test_nested_functions_and_scope() {
    Value v = run_and_get_result(
        "var x = 3; "
        "func outer(): int { "
        "    var x = 10; "
        "    func inner(): int { "
        "        return x + 1; "
        "    } "
        "    return inner(); "
        "} "
        "outer();"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 11, "Nested function must capture outer's x (10)");
    
    // Also verify global x is unchanged
    Value v2 = run_and_get_result(
        "var x = 3; "
        "func outer(): int { "
        "    var x = 10; "
        "    func inner(): int { "
        "        return x + 1; "
        "    } "
        "    return inner(); "
        "} "
        "outer(); "  // Call the function
        "x;"         // Return global x
    );
    
    CHECK(IS_INT(v2), "Global x must be integer");
    CHECK(AS_INT(v2) == 3, "Global x must remain unchanged");
}


/* -------------------------------------------------------------
 * TEST 6: Function return value (multiple calls)
 * ------------------------------------------------------------- */
static void test_function_return_value() {
    Value v = run_and_get_result(
        "func multiply(a, b): int { "
        "    return a * b; "
        "} "
        "multiply(6, 7);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 42, "multiply(6, 7) must return 42");
}


/* -------------------------------------------------------------
 * TEST 7: Function with local variables
 * ------------------------------------------------------------- */
static void test_function_with_locals() {
    Value v = run_and_get_result(
        "func compute(a, b, c): int { "
        "    var x = a + b; "
        "    var y = x * c; "
        "    return y; "
        "} "
        "compute(2, 3, 4);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 20, "compute(2,3,4) should return 20");
}


/* -------------------------------------------------------------
 * TEST 8: Functions with different arities
 * ------------------------------------------------------------- */
static void test_mixed_arity_calls() {
    Value v = run_and_get_result(
        "func add2(a, b): int { "
        "    return a + b; "
        "} "
        "func add3(a, b, c): int { "
        "    return add2(a, b) + c; "
        "} "
        "add3(1, 2, 3);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 6, "add3(1,2,3) should return 6");
}


/* -------------------------------------------------------------
 * TEST 9: Zero-argument function
 * ------------------------------------------------------------- */
static void test_zero_arg_function() {
    Value v = run_and_get_result(
        "func get_answer(): int { "
        "    return 42; "
        "} "
        "get_answer();"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 42, "get_answer() should return 42");
}


/* -------------------------------------------------------------
 * TEST 10: Function with early return in conditionals
 * ------------------------------------------------------------- */
static void test_early_return() {
    Value v = run_and_get_result(
        "func check(n): int { "
        "    if (n < 0) { "
        "        return -1; "
        "    } "
        "    if (n == 0) { "
        "        return 0; "
        "    } "
        "    return 1; "
        "} "
        "check(-5);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == -1, "check(-5) should return -1");
}


/* -------------------------------------------------------------
 * TEST 11: Function parameters shadow globals
 * ------------------------------------------------------------- */
static void test_param_shadowing() {
    Value v = run_and_get_result(
        "var x = 100; "
        "func test(x): int { "
        "    return x * 2; "
        "} "
        "test(5);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 10, "Function param 'x' should shadow global");
}


/* -------------------------------------------------------------
 * TEST 12: Sequential function calls
 * ------------------------------------------------------------- */
static void test_sequential_calls() {
    Value v = run_and_get_result(
        "func square(x): int { "
        "    return x * x; "
        "} "
        "square(3); "   // Result discarded (becomes expr statement)
        "square(4);"     // This is the final result
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 16, "Last call square(4) should return 16");
}


/* -------------------------------------------------------------
 * TEST 13: Function with complex expression
 * ------------------------------------------------------------- */
static void test_complex_expression() {
    Value v = run_and_get_result(
        "func calc(a, b, c): int { "
        "    return (a + b) * c - a; "
        "} "
        "calc(2, 3, 4);"
    );
    
    CHECK(IS_INT(v), "Result must be integer");
    CHECK(AS_INT(v) == 18, "(2+3)*4-2 should equal 18");
}


/* -------------------------------------------------------------
 * SUITE WRAPPER
 * ------------------------------------------------------------- */
void test_functions_suite() {
    run_test(test_add_function,                "Functions - add(a,b)");
    run_test(test_twice_quad,                  "Functions - twice + quad");
    run_test(test_recursion_fib,               "Functions - recursive fib");
    run_test(test_scoping,                     "Functions - variable scoping");
    run_test(test_nested_functions_and_scope,  "Functions - nested lexical scoping");
    run_test(test_function_return_value,       "Functions - return value");
    run_test(test_function_with_locals,        "Functions - parameters and locals");
    run_test(test_mixed_arity_calls,           "Functions - mixed arity calls");
    run_test(test_zero_arg_function,           "Functions - zero arguments");
    run_test(test_early_return,                "Functions - early return");
    run_test(test_param_shadowing,             "Functions - parameter shadowing");
    run_test(test_sequential_calls,            "Functions - sequential calls");
    run_test(test_complex_expression,          "Functions - complex expression");
}