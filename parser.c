#include "compiler.h"

// Forward declarations for all static functions
static void parser_eat(Parser* parser, TokenType type);
static ASTNode* parse_number(Parser* parser);
static ASTNode* parse_identifier(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_factor(Parser* parser);
static ASTNode* parse_term(Parser* parser);
static ASTNode* parse_function_call(Parser* parser, char* function_name);
static ASTNode* parse_compound_statement(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_if_statement(Parser* parser);
static ASTNode* parse_while_statement(Parser* parser);
static ASTNode* parse_variable_declaration(Parser* parser);
static ASTNode* parse_return_statement(Parser* parser);
static ASTNode* parse_function_declaration(Parser* parser);

Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = get_next_token(lexer);
    return parser;
}

static void parser_eat(Parser* parser, TokenType type) {
    if (parser->current_token->type == type) {
        Token* old_token = parser->current_token;
        parser->current_token = get_next_token(parser->lexer);
        free(old_token->value);
        free(old_token);
    } else {
        fprintf(stderr, "Unexpected token type: %d\n", parser->current_token->type);
        exit(1);
    }
}

static ASTNode* parse_number(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.number_value = atoi(parser->current_token->value);
    parser_eat(parser, TOKEN_NUMBER);
    return node;
}

static ASTNode* parse_identifier(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->data.string_value = strdup(parser->current_token->value);
    parser_eat(parser, TOKEN_IDENTIFIER);
    return node;
}

static ASTNode* parse_expression(Parser* parser);

static ASTNode* parse_factor(Parser* parser) {
    Token* token = parser->current_token;
    
    if (token->type == TOKEN_NUMBER) {
        return parse_number(parser);
    } else if (token->type == TOKEN_IDENTIFIER) {
        char* name = strdup(token->value);
        parser_eat(parser, TOKEN_IDENTIFIER);
        
        // Check if it's a function call
        if (parser->current_token->type == TOKEN_LPAREN) {
            return parse_function_call(parser, name);
        }
        
        // It's a variable
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->data.string_value = name;
        return node;
    } else if (token->type == TOKEN_LPAREN) {
        parser_eat(parser, TOKEN_LPAREN);
        ASTNode* node = parse_expression(parser);
        parser_eat(parser, TOKEN_RPAREN);
        return node;
    }
    
    fprintf(stderr, "Unexpected token in factor\n");
    exit(1);
}

static ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_factor(parser);
    
    while (parser->current_token->type == TOKEN_MULTIPLY || 
           parser->current_token->type == TOKEN_DIVIDE) {
        Token* token = parser->current_token;
        if (token->type == TOKEN_MULTIPLY) {
            parser_eat(parser, TOKEN_MULTIPLY);
        } else if (token->type == TOKEN_DIVIDE) {
            parser_eat(parser, TOKEN_DIVIDE);
        }
        
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.binary.operator = token->type == TOKEN_MULTIPLY ? '*' : '/';
        new_node->data.binary.left = node;
        new_node->data.binary.right = parse_factor(parser);
        node = new_node;
    }
    
    return node;
}

static ASTNode* parse_expression(Parser* parser) {
    ASTNode* node = parse_term(parser);
    
    while (parser->current_token->type == TOKEN_PLUS || 
           parser->current_token->type == TOKEN_MINUS) {
        Token* token = parser->current_token;
        if (token->type == TOKEN_PLUS) {
            parser_eat(parser, TOKEN_PLUS);
        } else if (token->type == TOKEN_MINUS) {
            parser_eat(parser, TOKEN_MINUS);
        }
        
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.binary.operator = token->type == TOKEN_PLUS ? '+' : '-';
        new_node->data.binary.left = node;
        new_node->data.binary.right = parse_term(parser);
        node = new_node;
    }
    
    return node;
}

static ASTNode* parse_function_declaration(Parser* parser) {
    // Expect: int function_name(int param1, int param2) { ... }
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DECLARATION;
    
    // Parse return type
    parser_eat(parser, TOKEN_KEYWORD); // 'int'
    
    // Parse function name
    node->data.function.name = strdup(parser->current_token->value);
    parser_eat(parser, TOKEN_IDENTIFIER);
    
    // Parse parameters
    parser_eat(parser, TOKEN_LPAREN);
    
    node->data.function.parameters = malloc(sizeof(ASTNode*) * 10); // Max 10 parameters
    node->data.function.parameter_count = 0;
    
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (node->data.function.parameter_count > 0) {
            parser_eat(parser, TOKEN_COMMA);
        }
        
        ASTNode* param = (ASTNode*)malloc(sizeof(ASTNode));
        param->type = NODE_VARIABLE_DECLARATION;
        
        // Parse parameter type
        parser_eat(parser, TOKEN_KEYWORD); // 'int'
        
        // Parse parameter name
        param->data.variable.name = strdup(parser->current_token->value);
        param->data.variable.type = strdup("int");
        parser_eat(parser, TOKEN_IDENTIFIER);
        
        node->data.function.parameters[node->data.function.parameter_count++] = param;
    }
    
    parser_eat(parser, TOKEN_RPAREN);
    
    // Parse function body
    parser_eat(parser, TOKEN_LBRACE);
    node->data.function.body = parse_compound_statement(parser);
    parser_eat(parser, TOKEN_RBRACE);
    
    return node;
}

static ASTNode* parse_compound_statement(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_COMPOUND_STATEMENT;
    node->data.block.statements = malloc(sizeof(ASTNode*) * 100); // Max 100 statements
    node->data.block.statement_count = 0;
    
    while (parser->current_token->type != TOKEN_RBRACE) {
        node->data.block.statements[node->data.block.statement_count++] = parse_statement(parser);
    }
    
    return node;
}

static ASTNode* parse_statement(Parser* parser);

static ASTNode* parse_if_statement(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    
    parser_eat(parser, TOKEN_IF);
    parser_eat(parser, TOKEN_LPAREN);
    
    // Parse condition
    node->data.if_statement.condition = parse_expression(parser);
    
    parser_eat(parser, TOKEN_RPAREN);
    parser_eat(parser, TOKEN_LBRACE);
    
    // Parse if body
    node->data.if_statement.if_body = parse_compound_statement(parser);
    
    parser_eat(parser, TOKEN_RBRACE);
    
    // Check for else
    if (parser->current_token->type == TOKEN_ELSE) {
        parser_eat(parser, TOKEN_ELSE);
        parser_eat(parser, TOKEN_LBRACE);
        node->data.if_statement.else_body = parse_compound_statement(parser);
        parser_eat(parser, TOKEN_RBRACE);
    } else {
        node->data.if_statement.else_body = NULL;
    }
    
    return node;
}

static ASTNode* parse_while_statement(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    
    parser_eat(parser, TOKEN_WHILE);
    parser_eat(parser, TOKEN_LPAREN);
    
    // Parse condition
    node->data.while_statement.condition = parse_expression(parser);
    
    parser_eat(parser, TOKEN_RPAREN);
    parser_eat(parser, TOKEN_LBRACE);
    
    // Parse body
    node->data.while_statement.body = parse_compound_statement(parser);
    
    parser_eat(parser, TOKEN_RBRACE);
    
    return node;
}

static ASTNode* parse_variable_declaration(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE_DECLARATION;
    
    // Parse type (currently only supporting 'int')
    parser_eat(parser, TOKEN_KEYWORD);  // 'int'
    node->data.variable.type = strdup("int");
    
    // Parse variable name
    node->data.variable.name = strdup(parser->current_token->value);
    parser_eat(parser, TOKEN_IDENTIFIER);
    
    // Check for initialization
    if (parser->current_token->type == TOKEN_ASSIGN) {
        parser_eat(parser, TOKEN_ASSIGN);
        node->data.variable.initializer = parse_expression(parser);
    } else {
        node->data.variable.initializer = NULL;
    }
    
    parser_eat(parser, TOKEN_SEMICOLON);
    return node;
}

static ASTNode* parse_return_statement(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    
    parser_eat(parser, TOKEN_RETURN);
    node->data.binary.left = parse_expression(parser);  // Using binary.left to store return value
    parser_eat(parser, TOKEN_SEMICOLON);
    
    return node;
}

static ASTNode* parse_statement(Parser* parser) {
    switch (parser->current_token->type) {
        case TOKEN_IF:
            return parse_if_statement(parser);
        case TOKEN_WHILE:
            return parse_while_statement(parser);
        case TOKEN_KEYWORD:  // int (variable declaration)
            return parse_variable_declaration(parser);
        case TOKEN_RETURN:
            return parse_return_statement(parser);
        case TOKEN_IDENTIFIER: {
            Token* next = get_next_token(parser->lexer);  // Peek next token
            if (next->type == TOKEN_ASSIGN) {
                // Assignment statement
                ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
                node->type = NODE_ASSIGNMENT;
                node->data.binary.left = parse_identifier(parser);
                parser_eat(parser, TOKEN_ASSIGN);
                node->data.binary.right = parse_expression(parser);
                parser_eat(parser, TOKEN_SEMICOLON);
                return node;
            } else {
                // Expression statement (function call)
                ASTNode* expr = parse_expression(parser);
                parser_eat(parser, TOKEN_SEMICOLON);
                return expr;
            }
        }
        default:
            fprintf(stderr, "Unexpected token in statement: %d\n", parser->current_token->type);
            exit(1);
    }
}

static ASTNode* parse_function_call(Parser* parser, char* function_name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->data.function.name = strdup(function_name);
    
    // Parse arguments
    parser_eat(parser, TOKEN_LPAREN);
    
    node->data.function.parameters = malloc(sizeof(ASTNode*) * 10); // Max 10 arguments
    node->data.function.parameter_count = 0;
    
    while (parser->current_token->type != TOKEN_RPAREN) {
        if (node->data.function.parameter_count > 0) {
            parser_eat(parser, TOKEN_COMMA);
        }
        
        node->data.function.parameters[node->data.function.parameter_count++] = 
            parse_expression(parser);
    }
    
    parser_eat(parser, TOKEN_RPAREN);
    return node;
}

ASTNode* parse(Parser* parser) {
    ASTNode* program = (ASTNode*)malloc(sizeof(ASTNode));
    program->type = NODE_PROGRAM;
    program->data.block.statements = malloc(sizeof(ASTNode*) * 100);
    program->data.block.statement_count = 0;
    
    while (parser->current_token->type != TOKEN_EOF) {
        if (parser->current_token->type == TOKEN_KEYWORD) {
            program->data.block.statements[program->data.block.statement_count++] = 
                parse_function_declaration(parser);
        }
    }
    
    return program;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            // Assuming node->data.block.statements is an array of ASTNode*
            for (int i = 0; i < node->data.block.statement_count; i++) {
                free_ast(node->data.block.statements[i]); // Free each statement
            }
            free(node->data.block.statements); // Free the array itself
            break;
        case NODE_FUNCTION:
            // Handle NODE_FUNCTION
            break;
        case NODE_DECLARATION:
            // Handle NODE_DECLARATION
            break;
        case NODE_ASSIGNMENT:
            // Handle NODE_ASSIGNMENT
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right);
            break;
        case NODE_NUMBER:
            // Handle NODE_NUMBER
            break;
        case NODE_RETURN:
            // Handle NODE_RETURN
            free_ast(node->data.binary.left);
            break;
        case NODE_FUNCTION_CALL:
            // Handle NODE_FUNCTION_CALL
            break;
        case NODE_COMPARISON:
            // Handle NODE_COMPARISON
            break;
        // Add other cases as necessary
        default:
            // Handle unexpected node types
            break;
    }
    
    free(node); // Free the current node
}

void free_parser(Parser* parser) {
    free(parser);
} 