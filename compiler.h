#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,
    TOKEN_KEYWORD,
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_COMMA,     // ,
    TOKEN_EQUALS,    // ==
    TOKEN_NOT_EQUALS, // !=
    TOKEN_LESS,      // <
    TOKEN_GREATER,   // >
    TOKEN_LESS_EQUALS,    // <=
    TOKEN_GREATER_EQUALS, // >=
    TOKEN_WHILE,     // while keyword
    TOKEN_IF,        // if keyword
    TOKEN_ELSE,      // else keyword
    TOKEN_RETURN     // return keyword
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* value;
} Token;

// Lexer structure
typedef struct {
    char* source;
    int position;
    int length;
} Lexer;

// AST Node Types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_RETURN,
    NODE_IF,
    NODE_WHILE,
    NODE_COMPOUND_STATEMENT,
    NODE_VARIABLE_DECLARATION,
    NODE_FUNCTION_DECLARATION,
    NODE_FUNCTION_CALL,
    NODE_COMPARISON
} NodeType;

// AST Node Structure
typedef struct ASTNode {
    NodeType type;
    union {
        // For numbers
        int number_value;
        // For identifiers and operators
        char* string_value;
        // For binary operations
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char operator;
        } binary;
        // For blocks and function bodies
        struct {
            struct ASTNode** statements;
            int statement_count;
        } block;
        struct {
            char* name;
            struct ASTNode** parameters;
            int parameter_count;
            struct ASTNode* body;
        } function;
        struct {
            char* name;
            char* type;
            struct ASTNode* initializer;
        } variable;
        struct {
            struct ASTNode* condition;
            struct ASTNode* if_body;
            struct ASTNode* else_body;
        } if_statement;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_statement;
    } data;
} ASTNode;

// Parser structure
typedef struct {
    Lexer* lexer;
    Token* current_token;
} Parser;

// Function declarations
Lexer* create_lexer(char* source);
Token* get_next_token(Lexer* lexer);
void free_lexer(Lexer* lexer);

// Parser function declarations
Parser* create_parser(Lexer* lexer);
ASTNode* parse(Parser* parser);
void free_parser(Parser* parser);
void free_ast(ASTNode* node);

// Debug printing functions
void print_token(Token* token);
void print_ast(ASTNode* node, int indent);

// Add to the existing declarations
const char* get_token_name(TokenType type) {
    static const char* token_names[] = {
        "EOF",
        "IDENTIFIER",
        "NUMBER",
        "PLUS",
        "MINUS",
        "MULTIPLY",
        "DIVIDE",
        "LPAREN",
        "RPAREN",
        "SEMICOLON",
        "ASSIGN",
        "KEYWORD",
        "LBRACE",
        "RBRACE",
        "COMMA",
        "EQUALS",
        "NOT_EQUALS",
        "LESS",
        "GREATER",
        "WHILE",
        "IF",
        "ELSE",
        "RETURN"
    };
    return token_names[type];
}

#endif 