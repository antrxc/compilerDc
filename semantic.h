#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "compiler.h"
#include <stdbool.h>

typedef struct Symbol {
    char* name;
    char* type;
    int scope_level;
    bool is_function;
    struct {
        char** param_types;
        int param_count;
    } function_data;
} Symbol;

typedef struct SymbolTable {
    Symbol* symbols;
    int count;
    int capacity;
    int current_scope;
} SymbolTable;

typedef struct {
    SymbolTable* table;
    char* current_function;
    bool has_return;
    char* error_message;
} SemanticAnalyzer;

SemanticAnalyzer* create_analyzer(void);
bool analyze(SemanticAnalyzer* analyzer, ASTNode* ast);
void free_analyzer(SemanticAnalyzer* analyzer);
const char* get_semantic_error(SemanticAnalyzer* analyzer);

#endif 