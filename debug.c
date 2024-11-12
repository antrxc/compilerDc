#include "compiler.h"

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void print_token(Token* token) {
    const char* type_names[] = {
        "EOF", "IDENTIFIER", "NUMBER", "PLUS", "MINUS", "MULTIPLY", 
        "DIVIDE", "LPAREN", "RPAREN", "SEMICOLON", "ASSIGN", "KEYWORD",
        "LBRACE", "RBRACE", "COMMA", "EQUALS", "NOT_EQUALS", "LESS",
        "GREATER", "WHILE", "IF", "ELSE", "RETURN"
    };
    
    printf("Token { type: %-12s, value: '%s' }\n", 
           type_names[token->type], 
           token->value ? token->value : "null");
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.block.statement_count; i++) {
                print_ast(node->data.block.statements[i], indent + 1);
            }
            break;
            
        case NODE_FUNCTION_DECLARATION:
            printf("Function: %s\n", node->data.function.name);
            print_indent(indent + 1);
            printf("Parameters:\n");
            for (int i = 0; i < node->data.function.parameter_count; i++) {
                print_ast(node->data.function.parameters[i], indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.function.body, indent + 2);
            break;
            
        case NODE_VARIABLE_DECLARATION:
            printf("VarDecl: %s (type: %s)\n", 
                   node->data.variable.name, 
                   node->data.variable.type);
            if (node->data.variable.initializer) {
                print_ast(node->data.variable.initializer, indent + 1);
            }
            break;
            
        case NODE_NUMBER:
            printf("Number: %d\n", node->data.number_value);
            break;
            
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->data.string_value);
            break;
            
        case NODE_BINARY_OP:
            printf("BinaryOp: %c\n", node->data.binary.operator);
            print_ast(node->data.binary.left, indent + 1);
            print_ast(node->data.binary.right, indent + 1);
            break;
            
        case NODE_IF:
            printf("If\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.if_statement.condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_ast(node->data.if_statement.if_body, indent + 2);
            if (node->data.if_statement.else_body) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_ast(node->data.if_statement.else_body, indent + 2);
            }
            break;
            
        case NODE_WHILE:
            printf("While\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.while_statement.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.while_statement.body, indent + 2);
            break;
            
        case NODE_RETURN:
            printf("Return\n");
            print_ast(node->data.binary.left, indent + 1);
            break;
            
        case NODE_FUNCTION_CALL:
            printf("FunctionCall: %s\n", node->data.function.name);
            print_indent(indent + 1);
            printf("Arguments:\n");
            for (int i = 0; i < node->data.function.parameter_count; i++) {
                print_ast(node->data.function.parameters[i], indent + 2);
            }
            break;
            
        default:
            printf("Unknown node type: %d\n", node->type);
    }
} 