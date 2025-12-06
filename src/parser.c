/**
 * @file parser.c
 * @brief Implementation of the Determa Parser
 *
 * This is a recursive descent parser that builds the AST
 * 
 * 
 * PRECIDENCE HEIRARCHY (As of v0.2)
 * 
    Equality (==, !=)

    Comparison (<, >, <=, >=)

    Term (+, -)

    Factor (*, /)

    Unary (!, -)

    Primary (true, false, 123, (...))
 */

#include "parser.h"
#include "ast.h"
#include "token.h"
#include "lexer.h"
#include "debug.h" // for PDA Logging

#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy


// --- Globals for PDA Debugger ---
int pda_debug_enabled = 0;
int pda_debug_indent = 0;

// --- Parser Globals (State) ---

/**
 * @struct Parser
 * @brief Holds the state of the parser as it consumes tokens
 *
 * This is the practical implementation of our "PDA".
 * The C call stack acts as the PDA's stack.
 */
typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int had_error;
    // We will add a 'panic_mode' flag later for error recovery
} Parser;

// --- Forward Declarations for recursive functions ---
static AstNode* parse_assignment(Parser* parser);
static AstNode* parse_expression(Parser* parser);
static AstNode* parse_equality(Parser* parser);
static AstNode* parse_comparison(Parser* parser);
static AstNode* parse_term(Parser* parser);
static AstNode* parse_factor(Parser* parser);
static AstNode* parse_unary(Parser* parser);
static AstNode* parse_primary(Parser* parser);
static AstNode* parse_declaration(Parser* parser);
static AstNode* parse_statement(Parser* parser);
static AstNode* parse_block(Parser* parser);
static AstNode* parse_if_statement(Parser* parser);
static AstNode* parse_function_declaration(Parser* parser);
static AstNode* parse_return_statement(Parser* parser);

// --- Error Handling & Token Helpers ---

/**
 * @brief Prints a parser error message
 */
static void error_at_current(Parser* parser, const char* message) {
    parser->had_error = 1;
    // Don't print if we've already hit an error
    // (This prevents a cascade of errors)
    // We will implement panic_mode later
    
    printf("[Line %d] Error", parser->current.line);
    if (parser->current.type == TOKEN_EOF) {
        printf(" at end");
    } else if (parser->current.type != TOKEN_ERROR) {
        printf(" at '%.*s'", parser->current.length, parser->current.lexeme);
    }
    printf(": %s\n", message);
}

/**
 * @brief Consumes the next token from the lexer
 */
static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    // Keep scanning until we get a non-error token
    for (;;) {
        parser->current = get_next_token(&parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        // Report the lexer error
        error_at_current(parser, parser->current.lexeme);
    }
}

/**
 * @brief Consumes the current token if it matches the expected type
 *
 * If it matches, advance to the next token
 * If not, report an error
 */
static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    error_at_current(parser, message);
}

/**
 * @brief Checks if the current token is of a given type
 */
static int check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

/**
 * @brief Checks if the current token matches any in a list
 *
 * This is a helper for binary operators
 */
static int match(Parser* parser, TokenType types[], int count) {
    for (int i = 0; i < count; i++) {
        if (check(parser, types[i])) {
            advance(parser);
            return 1;
        }
    }
    return 0;
}



// --- Parsing Functions (The Grammar) ---

/**
 * @brief Parses a "primary" expression (the lowest level)
 *
 * Grammar Rule:
 * primary -> NUMBER | "(" expression ")"
 */
static AstNode* parse_primary(Parser* parser) {
    TRACE_ENTER("Primary");
    // Check true boolean parsing
    if (check(parser, TOKEN_TRUE)) {
        advance(parser);
        TRACE_EXIT("Primary (Bool)");
        return new_bool_literal_node(1, parser->previous.line);
    }

    // Check false boolean parsing
    if (check(parser, TOKEN_FALSE)) {
        advance(parser);
        TRACE_EXIT("Primary (Bool)");
        return new_bool_literal_node(0, parser->previous.line);
    }


    // It's a number literal
    if (check(parser, TOKEN_INT)) {
        char buf[64];
        int len = parser->current.length;
        if (len >= 63) len = 63; // Truncate if too long

        memcpy(buf, parser->current.lexeme, len);
        buf[len] = '\0';

        long val = strtol(buf, NULL, 10);
        advance(parser);
        TRACE_EXIT("Primary (IntLiteral)");
        return new_int_literal_node((int)val, parser->previous.line);
    }

    // --- String Literal Parsing ---
    if (check(parser, TOKEN_STRING)) {

        if (parser->current.length < 2) {
            error_at_current(parser, "Invalid string literal");
            advance(parser);
            return NULL;
        }

        // strip the token lexemme of the " "
        int len = parser->current.length - 2; // -2 for quotes
        char* strVal = (char*)malloc(len + 1);
        // Copy starting from lexeme + 1 to skip opening quote
        if (!strVal) { error_at_current(parser, "Out of memory"); return NULL; }
        
        memcpy(strVal, parser->current.lexeme + 1, len);
        strVal[len] = '\0';
        
        advance(parser);
        TRACE_EXIT("Primary (StringLiteral)");
        return new_string_literal_node(strVal, parser->previous.line);
    }

    // Identifier: Var or Call
    if (check(parser, TOKEN_ID)) {
        Token name = parser->current;
        advance(parser);
        
        // Call?
        if (check(parser, TOKEN_LPAREN)) {
            advance(parser); // Consume (
            int line = parser->previous.line;
            int arg_capacity = 4;
            int arg_count = 0;
            AstNode** args = malloc(sizeof(AstNode*) * arg_capacity);
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    if (arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        args = realloc(args, sizeof(AstNode*) * arg_capacity);
                    }
                    args[arg_count++] = parse_expression(parser);
                } while (match(parser, (TokenType[]){TOKEN_COMMA}, 1));
            }
            consume(parser, TOKEN_RPAREN, "Expected ')' after args");
            TRACE_EXIT("Primary (Call)");
            return new_call_node(name, args, arg_count, line);
        }

        TRACE_EXIT("Primary (VarAccess)");
        return new_var_access_node(name, parser->previous.line);
    }

    // Consume '('
    if (check(parser, TOKEN_LPAREN)) {
        advance(parser); 
        AstNode* expr = parse_expression(parser); // Parse inner expression
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        TRACE_EXIT("Primary (Grouping)");
        return expr;
    }

    // If we get here, it's an error
    error_at_current(parser, "Expected expression");
    TRACE_EXIT("Primary (Error)");
    return NULL;
}

/**
 * @brief Parses unary expressions (-int, -x)
 * Rule: unary -> ("-") unary | primary
 */
static AstNode* parse_unary(Parser* parser) {
    TRACE_ENTER("Unary");

    // FIX: Explicitly match TOKEN_BANG (!) here
    TokenType ops[] = {TOKEN_MINUS, TOKEN_BANG};

    if (match(parser, ops, 2)) {
        Token operator = parser->previous;
        // recursively call unary
        AstNode* operand = parse_unary(parser);
        TRACE_EXIT("Unary (Op)");
        return new_unary_op_node(operator, operand, operator.line);
    }
    
    // If no unary op, fall through to primary
    AstNode* res = parse_primary(parser);
    TRACE_EXIT("Unary (Primary)");
    return res;
}

/**
 * @brief Parses "factor" expressions (multiplication, division)
 *
 * Grammar Rule:
 * factor -> primary ( ( "/" | "*" ) primary )*
 */
static AstNode* parse_factor(Parser* parser) {
    TRACE_ENTER("Factor");
    // calling parse_unary with a fallback to parse_primary
    AstNode* node = parse_unary(parser);

    TokenType op_types[] = {TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT};
    while (match(parser, op_types, 3)) {
        Token op = parser->previous;
        AstNode* right = parse_unary(parser); // right side also unary
        node = new_binary_op_node(op, node, right, op.line);
    }
    
    TRACE_EXIT("Factor");
    return node;
}

/**
 * @brief Parses "term" expressions (addition, subtraction)
 *
 * Grammar Rule:
 * term -> factor ( ( "-" | "+" ) factor )*
 */
static AstNode* parse_term(Parser* parser) {
    TRACE_ENTER("Term");
    AstNode* node = parse_factor(parser);

    TokenType op_types[] = {TOKEN_PLUS, TOKEN_MINUS};
    while (match(parser, op_types, 2)) {
        Token op = parser->previous;
        AstNode* right = parse_factor(parser);
        node = new_binary_op_node(op, node, right, op.line);
    }
    
    TRACE_EXIT("Term");
    return node;
}


/**
 * @brief Parses an expression (Handles precedence)
 *
 * Grammar Rule:
 * expression -> term
 */
static AstNode* parse_expression(Parser* parser) {
    TRACE_ENTER("Expression");
    // Start at the lowest precedence (Assignment)
    AstNode* node = parse_assignment(parser);
    TRACE_EXIT("Expression");
    return node;
}


// --- Comparison Parsing ---
static AstNode* parse_comparison(Parser* parser) {
    TRACE_ENTER("Comparison");
    AstNode* node = parse_term(parser);

    TokenType op_types[] = {TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL};
    while (match(parser, op_types, 4)) {
        Token op = parser->previous;
        AstNode* right = parse_term(parser);
        node = new_binary_op_node(op, node, right, op.line);
    }
    
    TRACE_EXIT("Comparison");
    return node;
}

static AstNode* parse_equality(Parser* parser) {
    TRACE_ENTER("Equality");
    AstNode* node = parse_comparison(parser);

    TokenType op_types[] = {TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL};
    while (match(parser, op_types, 2)) {
        Token op = parser->previous;
        AstNode* right = parse_comparison(parser);
        node = new_binary_op_node(op, node, right, op.line);
    }
    
    TRACE_EXIT("Equality");
    return node;
}

// --- Statement Parsing ---

/**
 * @brief Parses a print statement
 * 
 * Grammar:
 * @code
 print_statement → "print" expression ";"
 @endcode
 * 
 * @note assumes the 'print' keyword has already been consumed
 * by the caller. It parses the following expression, ensures that the
 * statement ends with a semicolon, and returns a fully constructed
 * `AstNodePrintStmt` node.
 *
 * PDA tracing is emitted when debug mode is enabled
 * 
 * @param parser Pointer to the parser instance
 * @return AstNode* new print-statement AST node
 */
static AstNode* parse_print_statement(Parser* parser) {
    TRACE_ENTER("PrintStmt");

    // Capture the line of the 'print' token (which was just consumed/matched in parse_statement)
    int line = parser->previous.line; 

    AstNode* value = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after value");
    TRACE_EXIT("PrintStmt");
    return new_print_stmt_node(value, line);
}

/**
 * @brief Parses a variable declaration
 * 
 * @details Grammar:
 * @code
 * var_declaration → IDENTIFIER ( "=" expression )? ";"
 * @endcode
 * 
 * @param parser Pointer to the parser instance
 * @return AstNode* variable-declaration AST node
 */
static AstNode* parse_var_declaration(Parser* parser) {
    TRACE_ENTER("VarDecl");
    // 'var' token was previous
    int line = parser->previous.line;

    consume(parser, TOKEN_ID, "Expected variable name");
    Token name = parser->previous;
    
    AstNode* initializer = NULL;
    if (match(parser, (TokenType[]){TOKEN_EQUALS}, 1)) {
        initializer = parse_expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    TRACE_EXIT("VarDecl");
    return new_var_decl_node(name, initializer, line);
}


/**
 * @brief The top-level controller
 * 
 * It decides:
 * If the next token is var → call parse_var_declaration
 * Otherwise → treat it as a statement and call parse_statement
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_declaration(Parser* parser) {
    TRACE_ENTER("Declaration");

    if (match(parser, (TokenType[]){TOKEN_FUNC}, 1)) {
        AstNode* decl = parse_function_declaration(parser);
        TRACE_EXIT("Declaration (func)");
        return decl;
    }
    
    if (match(parser, (TokenType[]){TOKEN_VAR}, 1)) {
        AstNode* decl = parse_var_declaration(parser);
        TRACE_EXIT("Declaration (Var)");
        return decl;
    }
    AstNode* stmt = parse_statement(parser);
    TRACE_EXIT("Declaration (Stmt)");
    return stmt;
}


/**
 * @brief Parses Assignment logic when encountered
 * 
 * Rule: assignment -> IDENTIFIER "=" assignment | equality 
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_assignment(Parser* parser) {
    // 1. Parse the left side as an expression (likely a VarAccess)
    // We call the next level down (Equality)
    AstNode* expr = parse_equality(parser);

    TokenType assignOps[] = {
        TOKEN_EQUALS, 
        TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL, 
        TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL,
        TOKEN_PERCENT_EQUAL
    };

    if (match(parser, assignOps, 6)) {
        Token op = parser->previous;
        AstNode* value = parse_assignment(parser);

        if (expr->type == NODE_VAR_ACCESS) {
            Token name = ((AstNodeVarAccess*)expr)->name;

            // Handle Compound Assignment (Desugaring)
            if (op.type != TOKEN_EQUALS) {
                // Transform "x += 1" into "x = x + 1"
                
                // 1. Convert the assignment op to a binary op
                TokenType binOpType;
                switch (op.type) {
                    case TOKEN_PLUS_EQUAL:    binOpType = TOKEN_PLUS; break;
                    case TOKEN_MINUS_EQUAL:   binOpType = TOKEN_MINUS; break;
                    case TOKEN_STAR_EQUAL:    binOpType = TOKEN_STAR; break;
                    case TOKEN_SLASH_EQUAL:   binOpType = TOKEN_SLASH; break;
                    case TOKEN_PERCENT_EQUAL: binOpType = TOKEN_PERCENT; break;
                    default: binOpType = TOKEN_PLUS; break; // Unreachable
                }
                
                // 2. Create the synthetic binary operator node (x + 1)
                // Reuse 'expr' (VarAccess x) as the left side
                Token binToken = op;
                binToken.type = binOpType;
                AstNode* binOpNode = new_binary_op_node(binToken, expr, value, op.line);
                
                // 3. Assign the result back to x
                return new_var_assign_node(name, binOpNode, op.line);
            }

            // Standard Assignment (x = 1)
            // Discard the VarAccess node as we replace it with VarAssign
            // (Ideally free(expr) here, but for now we rely on GC/OS cleanup)
            return new_var_assign_node(name, value, op.line);
        }

        error_at_current(parser, "Invalid assignment target.");
    }

    return expr;
}


/**
 * @brief Parses a block of statements enclosed in curly braces.
 * Determa uses C-style braces `{ ... }` for blocks.
 * 
 * @param parser The parser instance.
 * @return AstNode* The block node containing the statement list.
 */
static AstNode* parse_block(Parser* parser) {
    TRACE_ENTER("Block");
    int line = parser->previous.line;
    AstNode* block = new_block_node(line);
    
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
    AstNode* stmt = parse_declaration(parser);
    if (stmt) {
        block_add_statement(block, stmt);
    } else {
        if (parser->had_error) {
            // recover: skip tokens until end of statement or end of block
            while (!check(parser, TOKEN_EOF) &&
                   !check(parser, TOKEN_SEMICOLON) &&
                   !check(parser, TOKEN_RIGHT_BRACE)) {
                advance(parser);
            }
            if (check(parser, TOKEN_SEMICOLON)) advance(parser);
                parser->had_error = 0;
            } else {
                break;
            }
        }
    }

    
    consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
    TRACE_EXIT("Block");
    return block;
}


/**
 * @brief Parses an if-statement with optional elif/else blocks.
 * 
 * Syntax: 
 * if condition { ... }
 * 
 * elif condition { ... }
 * 
 * else { ... }
 * 
 * @note Determa does not require parentheses around the condition.
 */
static AstNode* parse_if_statement(Parser* parser) {
    TRACE_ENTER("IfStmt");
    int line = parser->previous.line; 
    
    AstNode* condition = parse_expression(parser);
    
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after if condition.");
    AstNode* thenBranch = parse_block(parser);
    
    AstNode* elseBranch = NULL;
    if (match(parser, (TokenType[]){TOKEN_ELIF}, 1)) {
        elseBranch = parse_if_statement(parser); // Recursively parse 'elif' as new 'if'
    } 
    else if (match(parser, (TokenType[]){TOKEN_ELSE}, 1)) {
        consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after else.");
        elseBranch = parse_block(parser);
    }

    TRACE_EXIT("IfStmt");
    return new_if_node(condition, thenBranch, elseBranch, line);
}


/**
 * @brief Parses a while-loop.
 * 
 * Syntax: while condition { ... }
 */
static AstNode* parse_while_statement(Parser* parser) {
    TRACE_ENTER("WhileStmt");
    int line = parser->previous.line; 
    
    AstNode* condition = parse_expression(parser);
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' after while condition.");
    AstNode* body = parse_block(parser);
    
    TRACE_EXIT("WhileStmt");
    return new_while_node(condition, body, line);
}


/**
 * @brief Function to parse a return statement
 * 
 * Parses the return keyword, consumes it and returns the DataType AST node
 * 
 * Syntax: return expr;
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_return_statement(Parser* parser) {
    TRACE_ENTER("ReturnStmt"); int line = parser->previous.line;
    AstNode* val = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) val = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after return value");
    TRACE_EXIT("ReturnStmt"); return new_return_node(val, line);
}


/**
 * @brief Function to parse a func (function) declaration block
 * 
 * Syntax: func name(arg1, arg2, ...) : returnType { ... }
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_function_declaration(Parser* parser) {
    TRACE_ENTER("FuncDecl"); int line = parser->previous.line;
    consume(parser, TOKEN_ID, "Expected function name"); Token name = parser->previous;
    consume(parser, TOKEN_LPAREN, "Expected '('");
    
    int cap = 4, count = 0;
    Token* params = malloc(sizeof(Token) * cap);
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (count >= cap) { cap *= 2; params = realloc(params, sizeof(Token) * cap); }
            consume(parser, TOKEN_ID, "Expected param name");
            params[count++] = parser->previous;
        } while (match(parser, (TokenType[]){TOKEN_COMMA}, 1));
    }
    consume(parser, TOKEN_RPAREN, "Expected ')'");
    
    DataType retType = TYPE_VOID;
    if (match(parser, (TokenType[]){TOKEN_COLON}, 1)) {
        if (match(parser, (TokenType[]){TOKEN_TYPE_INT}, 1)) retType = TYPE_INT;
        else if (match(parser, (TokenType[]){TOKEN_TYPE_BOOL}, 1)) retType = TYPE_BOOL;
        else if (match(parser, (TokenType[]){TOKEN_TYPE_STRING}, 1)) retType = TYPE_STRING;
        else if (match(parser, (TokenType[]){TOKEN_TYPE_VOID}, 1)) retType = TYPE_VOID;
        else error_at_current(parser, "Invalid return type");
    }
    
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{'");
    AstNode* body = parse_block(parser);
    TRACE_EXIT("FuncDecl");
    return new_func_decl_node(name, params, count, retType, body, line);
}

/**
 * @brief Handles executable statements (non-declaration)
 * 
 * eg: 
 * @code 
    print x + 1;
    x + y * 2;
    a = b + c;   // If assignment handled later
   @endcode
 * 
 * @param parser 
 * @return AstNode* 
 */
static AstNode* parse_statement(Parser* parser) {
    TRACE_ENTER("Statement");

    if (match(parser, (TokenType[]){TOKEN_IF}, 1)) {
        AstNode* stmt = parse_if_statement(parser);
        TRACE_EXIT("Statement (if)");
        return stmt;
    }

    if (match(parser, (TokenType[]){TOKEN_WHILE}, 1)) {
        AstNode* stmt = parse_while_statement(parser);
        TRACE_EXIT("Statement (while)");
        return stmt;
    }
    if (match(parser, (TokenType[]){TOKEN_LEFT_BRACE}, 1)) {
        AstNode* stmt = parse_block(parser);
        TRACE_EXIT("Statement (block)");
        return stmt;
    }

    if (match(parser, (TokenType[]){TOKEN_RETURN}, 1)) {
        AstNode* stmt = parse_return_statement(parser);
        TRACE_EXIT("Statement(return)");
        return stmt;
    }

    if (match(parser, (TokenType[]){TOKEN_PRINT}, 1)) {
        AstNode* stmt = parse_print_statement(parser);
        TRACE_EXIT("Statement (Print)");
        return stmt;
    }
    
    // If it's not a print statement, assume it's an expression statement
    AstNode* expr = parse_expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    TRACE_EXIT("Statement (Expr)");
    
    // Use expression's line for the statement line
    return new_expr_stmt_node(expr, expr ? expr->line : parser->previous.line);
}

/**
 * @brief The main function to parse source code
 *
 * This function initializes a parser and begins the parsing process
 *
 * @param source The source code string to parse
 * @param pda_debug_mode 0 to run silently, 1 to enable PDA trace logging
 * 
 * @return AstNode* The root of the generated AST (or NULL on error)
 */
AstNode* parse(const char* source, int pda_debug_mode) {
    /* ---- Debug Mode Setup ---- */
    pda_debug_enabled = pda_debug_mode;
    pda_debug_indent  = 0;

    /* ---- Init Parser State ---- */
    Parser parser;
    parser.lexer = init_lexer(source);
    parser.had_error = 0;

    // initialize tokens to safe defaults
    parser.current = (Token){ .type = TOKEN_ERROR, .lexeme = NULL, .length = 0, .line = 0 };
    parser.previous = (Token){ .type = TOKEN_ERROR, .lexeme = NULL, .length = 0, .line = 0 };


    // Load first token
    advance(&parser);

    // Create Program Node 
    AstNode* program = new_program_node(NULL, 0);

    if (!program) {
        fprintf(stderr, "Fatal: Failed to allocate program node.\n");
        return NULL;
    }

    // Parse Until EOF 
    while (!check(&parser, TOKEN_EOF)) {

        AstNode* stmt = parse_declaration(&parser);

        if (stmt != NULL) {
            program_add_statement(program, stmt);
        } else {
            // For now, if we hit an error, we might loop infinitely if we don't consume tokens.
            // A simple sync is to advance until semicolon.
            if (parser.had_error) {
                 // Simple panic mode recovery: skip until next statement
                 while (!check(&parser, TOKEN_EOF) && !check(&parser, TOKEN_SEMICOLON)) {
                     advance(&parser);
                 }
                 if (check(&parser, TOKEN_SEMICOLON)) advance(&parser);
                 parser.had_error = 0; // Reset error to try parsing next statement
            } else {
                break; 
            }
        }
    }

    // Final Error Check 
    if (parser.had_error) {
        free_ast(program);   // Clean up partial tree
        return NULL;
    }

    return program;  // Success
}