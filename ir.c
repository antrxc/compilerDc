#include "ir.h"
#include <stdarg.h>

IRProgram* create_ir_program(void) {
    IRProgram* program = malloc(sizeof(IRProgram));
    program->capacity = 1000;
    program->count = 0;
    program->temp_count = 0;
    program->label_count = 0;
    program->instructions = malloc(sizeof(IRInstr*) * program->capacity);
    return program;
}

char* new_temp(IRProgram* program) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "t%d", program->temp_count++);
    return strdup(buffer);
}

IRLabel* new_label(IRProgram* program) {
    IRLabel* label = malloc(sizeof(IRLabel));
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "L%d", program->label_count++);
    label->name = strdup(buffer);
    label->number = program->label_count - 1;
    return label;
}

void add_instruction(IRProgram* program, IRInstr* instr) {
    if (program->count >= program->capacity) {
        program->capacity *= 2;
        program->instructions = realloc(program->instructions, 
                                      sizeof(IRInstr*) * program->capacity);
    }
    program->instructions[program->count++] = instr;
}

static IRInstr* create_instr(IROpcode op, const char* dest, const char* src1, 
                            const char* src2) {
    IRInstr* instr = malloc(sizeof(IRInstr));
    instr->op = op;
    instr->dest = dest ? strdup(dest) : NULL;
    instr->src1 = src1 ? strdup(src1) : NULL;
    instr->src2 = src2 ? strdup(src2) : NULL;
    instr->label = NULL;
    return instr;
}

static char* generate_expression_ir(IRProgram* program, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER: {
            char* temp = new_temp(program);
            IRInstr* instr = create_instr(IR_ASSIGN, temp, NULL, NULL);
            instr->value = node->data.number_value;
            add_instruction(program, instr);
            return temp;
        }
        
        case NODE_IDENTIFIER:
            return strdup(node->data.string_value);
            
        case NODE_BINARY_OP: {
            char* left = generate_expression_ir(program, node->data.binary.left);
            char* right = generate_expression_ir(program, node->data.binary.right);
            char* result = new_temp(program);
            
            IROpcode op;
            switch (node->data.binary.operator) {
                case '+': op = IR_ADD; break;
                case '-': op = IR_SUB; break;
                case '*': op = IR_MUL; break;
                case '/': op = IR_DIV; break;
                default: fprintf(stderr, "Unknown operator\n"); exit(1);
            }
            
            add_instruction(program, create_instr(op, result, left, right));
            free(left);
            free(right);
            return result;
        }
        
        case NODE_FUNCTION_CALL: {
            // Generate code for arguments
            for (int i = 0; i < node->data.function.parameter_count; i++) {
                char* arg = generate_expression_ir(program, 
                                                node->data.function.parameters[i]);
                add_instruction(program, create_instr(IR_ARG, NULL, arg, NULL));
                free(arg);
            }
            
            // Generate call instruction
            char* result = new_temp(program);
            IRInstr* call = create_instr(IR_CALL, result, 
                                       node->data.function.name, NULL);
            call->value = node->data.function.parameter_count;
            add_instruction(program, call);
            return result;
        }
        
        default:
            fprintf(stderr, "Unknown expression type in IR generation\n");
            exit(1);
    }
}

static void generate_statement_ir(IRProgram* program, ASTNode* node) {
    switch (node->type) {
        case NODE_PROGRAM:
            // Handle NODE_PROGRAM
            break;
        case NODE_FUNCTION:
            // Handle NODE_FUNCTION
            break;
        case NODE_BLOCK:
            // Handle NODE_BLOCK
            break;
        case NODE_DECLARATION:
            // Handle NODE_DECLARATION
            break;
        case NODE_BINARY_OP:
            // Handle NODE_BINARY_OP
            break;
        case NODE_NUMBER:
            // Handle NODE_NUMBER
            break;
        case NODE_IDENTIFIER:
            // Handle NODE_IDENTIFIER
            break;
        case NODE_FUNCTION_DECLARATION:
            // Handle NODE_FUNCTION_DECLARATION
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
}

void generate_ir(IRProgram* program, ASTNode* ast) {
    if (ast->type != NODE_PROGRAM) {
        fprintf(stderr, "Expected program node\n");
        return;
    }
    
    // Generate IR for each function
    for (int i = 0; i < ast->data.block.statement_count; i++) {
        ASTNode* func = ast->data.block.statements[i];
        if (func->type == NODE_FUNCTION_DECLARATION) {
            // Function label
            IRInstr* label = create_instr(IR_LABEL, NULL, NULL, NULL);
            label->label = malloc(sizeof(IRLabel));
            label->label->name = strdup(func->data.function.name);
            label->label->number = -1;  // Special case for function labels
            add_instruction(program, label);
            
            // Parameters
            for (int j = 0; j < func->data.function.parameter_count; j++) {
                ASTNode* param = func->data.function.parameters[j];
                add_instruction(program, create_instr(IR_PARAM, 
                                                   param->data.variable.name, 
                                                   NULL, NULL));
            }
            
            // Function body
            generate_statement_ir(program, func->data.function.body);
        }
    }
}

void print_ir(IRProgram* program) {
    const char* opcode_names[] = {
        "ADD", "SUB", "MUL", "DIV", "ASSIGN", "LABEL", "JUMP",
        "JUMPZ", "JUMPNZ", "CALL", "RETURN", "PARAM", "ARG",
        "COMPARE", "LOAD", "STORE"
    };
    
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        
        if (instr->op == IR_LABEL) {
            printf("%s:\n", instr->label->name);
            continue;
        }
        
        printf("    %s ", opcode_names[instr->op]);
        
        if (instr->dest) printf("%s ", instr->dest);
        if (instr->src1) printf("%s ", instr->src1);
        if (instr->src2) printf("%s ", instr->src2);
        if (instr->label) printf("%s ", instr->label->name);
        if (instr->op == IR_ASSIGN) printf("%d", instr->value);
        
        printf("\n");
    }
}

void free_ir_program(IRProgram* program) {
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        free(instr->dest);
        free(instr->src1);
        free(instr->src2);
        if (instr->label) {
            free(instr->label->name);
            free(instr->label);
        }
        free(instr);
    }
    free(program->instructions);
    free(program);
} 