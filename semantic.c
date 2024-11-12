#include "semantic.h"
#include <stdarg.h>

static void set_error(SemanticAnalyzer* analyzer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (analyzer->error_message) {
        free(analyzer->error_message);
    }
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    analyzer->error_message = strdup(buffer);
    
    va_end(args);
}

static Symbol* add_symbol(SymbolTable* table, const char* name, const char* type) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(Symbol));
    }
    
    Symbol* symbol = &table->symbols[table->count++];
    symbol->name = strdup(name);
    symbol->type = strdup(type);
    symbol->scope_level = table->current_scope;
    symbol->is_function = false;
    return symbol;
}

static Symbol* find_symbol(SymbolTable* table, const char* name) {
    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

static void enter_scope(SymbolTable* table) {
    table->current_scope++;
}

static void exit_scope(SymbolTable* table) {
    // Remove all symbols from the current scope
    while (table->count > 0 && 
           table->symbols[table->count - 1].scope_level == table->current_scope) {
        table->count--;
        free(table->symbols[table->count].name);
        free(table->symbols[table->count].type);
    }
    table->current_scope--;
}

SemanticAnalyzer* create_analyzer(void) {
    SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
    analyzer->table = malloc(sizeof(SymbolTable));
    analyzer->table->capacity = 100;
    analyzer->table->count = 0;
    analyzer->table->current_scope = 0;
    analyzer->table->symbols = malloc(analyzer->table->capacity * sizeof(Symbol));
    analyzer->current_function = NULL;
    analyzer->has_return = false;
    analyzer->error_message = NULL;
    return analyzer;
}

static bool analyze_expression(SemanticAnalyzer* analyzer, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            return true;
            
        case NODE_IDENTIFIER: {
            Symbol* symbol = find_symbol(analyzer->table, node->data.string_value);
            if (!symbol) {
                set_error(analyzer, "Undefined variable: %s", node->data.string_value);
                return false;
            }
            return true;
        }
            
        case NODE_BINARY_OP:
            return analyze_expression(analyzer, node->data.binary.left) &&
                   analyze_expression(analyzer, node->data.binary.right);
            
        case NODE_FUNCTION_CALL: {
            Symbol* symbol = find_symbol(analyzer->table, node->data.function.name);
            if (!symbol || !symbol->is_function) {
                set_error(analyzer, "Undefined function: %s", node->data.function.name);
                return false;
            }
            
            if (symbol->function_data.param_count != node->data.function.parameter_count) {
                set_error(analyzer, "Wrong number of arguments for function %s", 
                         node->data.function.name);
                return false;
            }
            
            for (int i = 0; i < node->data.function.parameter_count; i++) {
                if (!analyze_expression(analyzer, node->data.function.parameters[i])) {
                    return false;
                }
            }
            return true;
        }
            
        default:
            return false;
    }
}

static bool analyze_statement(SemanticAnalyzer* analyzer, ASTNode* node) {
    switch (node->type) {
        case NODE_VARIABLE_DECLARATION: {
            if (find_symbol(analyzer->table, node->data.variable.name)) {
                set_error(analyzer, "Variable already declared: %s", 
                         node->data.variable.name);
                return false;
            }
            add_symbol(analyzer->table, node->data.variable.name, 
                      node->data.variable.type);
            
            if (node->data.variable.initializer) {
                return analyze_expression(analyzer, node->data.variable.initializer);
            }
            return true;
        }
            
        case NODE_ASSIGNMENT: {
            Symbol* symbol = find_symbol(analyzer->table, 
                                       node->data.binary.left->data.string_value);
            if (!symbol) {
                set_error(analyzer, "Assignment to undeclared variable: %s",
                         node->data.binary.left->data.string_value);
                return false;
            }
            return analyze_expression(analyzer, node->data.binary.right);
        }
            
        case NODE_IF:
            enter_scope(analyzer->table);
            bool result = analyze_expression(analyzer, node->data.if_statement.condition) &&
                         analyze_statement(analyzer, node->data.if_statement.if_body);
            if (node->data.if_statement.else_body) {
                result = result && analyze_statement(analyzer, 
                                                   node->data.if_statement.else_body);
            }
            exit_scope(analyzer->table);
            return result;
            
        case NODE_WHILE:
            enter_scope(analyzer->table);
            bool while_result = analyze_expression(analyzer, 
                                                 node->data.while_statement.condition) &&
                              analyze_statement(analyzer, node->data.while_statement.body);
            exit_scope(analyzer->table);
            return while_result;
            
        case NODE_RETURN:
            if (!analyzer->current_function) {
                set_error(analyzer, "Return statement outside of function");
                return false;
            }
            analyzer->has_return = true;
            return analyze_expression(analyzer, node->data.binary.left);
            
        case NODE_COMPOUND_STATEMENT:
            enter_scope(analyzer->table);
            for (int i = 0; i < node->data.block.statement_count; i++) {
                if (!analyze_statement(analyzer, node->data.block.statements[i])) {
                    return false;
                }
            }
            exit_scope(analyzer->table);
            return true;
            
        default:
            return analyze_expression(analyzer, node);
    }
}

bool analyze(SemanticAnalyzer* analyzer, ASTNode* ast) {
    if (ast->type != NODE_PROGRAM) {
        set_error(analyzer, "Root node must be a program");
        return false;
    }
    
    // First pass: register all function declarations
    for (int i = 0; i < ast->data.block.statement_count; i++) {
        ASTNode* node = ast->data.block.statements[i];
        if (node->type == NODE_FUNCTION_DECLARATION) {
            Symbol* symbol = add_symbol(analyzer->table, node->data.function.name, "function");
            symbol->is_function = true;
            symbol->function_data.param_count = node->data.function.parameter_count;
            symbol->function_data.param_types = 
                malloc(sizeof(char*) * node->data.function.parameter_count);
            
            for (int j = 0; j < node->data.function.parameter_count; j++) {
                symbol->function_data.param_types[j] = 
                    strdup(node->data.function.parameters[j]->data.variable.type);
            }
        }
    }
    
    // Second pass: analyze function bodies
    for (int i = 0; i < ast->data.block.statement_count; i++) {
        ASTNode* node = ast->data.block.statements[i];
        if (node->type == NODE_FUNCTION_DECLARATION) {
            analyzer->current_function = node->data.function.name;
            analyzer->has_return = false;
            
            enter_scope(analyzer->table);
            
            // Add parameters to symbol table
            for (int j = 0; j < node->data.function.parameter_count; j++) {
                ASTNode* param = node->data.function.parameters[j];
                add_symbol(analyzer->table, param->data.variable.name, 
                          param->data.variable.type);
            }
            
            if (!analyze_statement(analyzer, node->data.function.body)) {
                return false;
            }
            
            if (!analyzer->has_return && strcmp(node->data.function.name, "main") != 0) {
                set_error(analyzer, "Function %s must return a value", 
                         node->data.function.name);
                return false;
            }
            
            exit_scope(analyzer->table);
        }
    }
    
    return true;
}

void free_analyzer(SemanticAnalyzer* analyzer) {
    for (int i = 0; i < analyzer->table->count; i++) {
        free(analyzer->table->symbols[i].name);
        free(analyzer->table->symbols[i].type);
        if (analyzer->table->symbols[i].is_function) {
            for (int j = 0; j < analyzer->table->symbols[i].function_data.param_count; j++) {
                free(analyzer->table->symbols[i].function_data.param_types[j]);
            }
            free(analyzer->table->symbols[i].function_data.param_types);
        }
    }
    free(analyzer->table->symbols);
    free(analyzer->table);
    free(analyzer->error_message);
    free(analyzer);
}

const char* get_semantic_error(SemanticAnalyzer* analyzer) {
    return analyzer->error_message;
} 