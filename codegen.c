#include "codegen.h"
#include <stdarg.h>
#include <stdio.h>
#include "ir.h"


static void emit(CodeGenerator* gen, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(gen->output, fmt, args);
    va_end(args);
}

static IRLabel* new_label(IRProgram* program) {
    (void)program; // Prevent unused parameter warning
    // Function implementation...
    return NULL; // Ensure to return an appropriate value
}

static int get_variable_offset(CodeGenerator* gen, const char* name) {
    for (int i = 0; i < gen->variables.count; i++) {
        if (strcmp(gen->variables.names[i], name) == 0) {
            return gen->variables.offsets[i];
        }
    }
    // Add new variable
    gen->variables.names[gen->variables.count] = strdup(name);
    gen->stack_offset += 8;  // Assuming 64-bit integers
    gen->variables.offsets[gen->variables.count] = -gen->stack_offset;
    return -gen->stack_offset;
}

CodeGenerator* create_generator(const char* output_filename) {
    CodeGenerator* gen = malloc(sizeof(CodeGenerator));
    gen->output = fopen(output_filename, "w");
    gen->label_count = 0;
    gen->stack_offset = 0;
    gen->variables.names = malloc(sizeof(char*) * 100);  // Max 100 variables
    gen->variables.offsets = malloc(sizeof(int) * 100);
    gen->variables.count = 0;
    return gen;
}

static void generate_function_call(CodeGenerator* gen, ASTNode* node) {
    // Save registers that might be modified by the function call
    emit(gen, "    pushq %%rax\n");
    emit(gen, "    pushq %%rcx\n");
    emit(gen, "    pushq %%rdx\n");
    emit(gen, "    pushq %%rsi\n");
    emit(gen, "    pushq %%rdi\n");
    emit(gen, "    pushq %%r8\n");
    emit(gen, "    pushq %%r9\n");
    
    // Generate code for arguments in reverse order
    for (int i = node->data.function.parameter_count - 1; i >= 0; i--) {
        generate_expression(gen, node->data.function.parameters[i]);
        emit(gen, "    pushq %%rax\n");
    }
    
    // Pop arguments into the appropriate registers according to System V AMD64 ABI
    const char* arg_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    int reg_count = sizeof(arg_regs) / sizeof(arg_regs[0]);
    
    for (int i = 0; i < node->data.function.parameter_count && i < reg_count; i++) {
        emit(gen, "    popq %s\n", arg_regs[i]);
    }
    
    // Call the function
    emit(gen, "    call %s\n", node->data.function.name);
    
    // Restore saved registers in reverse order
    emit(gen, "    popq %%r9\n");
    emit(gen, "    popq %%r8\n");
    emit(gen, "    popq %%rdi\n");
    emit(gen, "    popq %%rsi\n");
    emit(gen, "    popq %%rdx\n");
    emit(gen, "    popq %%rcx\n");
    emit(gen, "    popq %%rbx\n");
}

static void generate_comparison(CodeGenerator* gen, ASTNode* node, const char* op) {
    generate_expression(gen, node->data.binary.right);
    emit(gen, "    pushq %%rax\n");
    generate_expression(gen, node->data.binary.left);
    emit(gen, "    popq %%rbx\n");
    emit(gen, "    cmpq %%rbx, %%rax\n");
    emit(gen, "    %s %%al\n", op);
    emit(gen, "    movzbq %%al, %%rax\n");
}

static void generate_expression(CodeGenerator* gen, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            emit(gen, "    movq $%d, %%rax\n", node->data.number_value);
            break;
            
        case NODE_IDENTIFIER:
            emit(gen, "    movq %d(%%rbp), %%rax\n", 
                 get_variable_offset(gen, node->data.string_value));
            break;
            
        case NODE_BINARY_OP:
            // Generate right operand first
            generate_expression(gen, node->data.binary.right);
            // Save right operand
            emit(gen, "    pushq %%rax\n");
            // Generate left operand
            generate_expression(gen, node->data.binary.left);
            // Restore right operand into rbx
            emit(gen, "    popq %%rbx\n");
            
            switch (node->data.binary.operator) {
                case '+':
                    emit(gen, "    addq %%rbx, %%rax\n");
                    break;
                case '-':
                    emit(gen, "    subq %%rbx, %%rax\n");
                    break;
                case '*':
                    emit(gen, "    imulq %%rbx, %%rax\n");
                    break;
                case '/':
                    emit(gen, "    cqto\n");           // Sign extend rax into rdx
                    emit(gen, "    idivq %%rbx\n");    // Divide rdx:rax by rbx
                    break;
            }
            break;
            
        case NODE_FUNCTION_CALL:
            generate_function_call(gen, node);
            break;
            
        case NODE_COMPARISON:
            switch (node->data.binary.operator) {
                case TOKEN_EQUALS:
                    generate_comparison(gen, node, "sete");
                    break;
                case TOKEN_NOT_EQUALS:
                    generate_comparison(gen, node, "setne");
                    break;
                case TOKEN_LESS:
                    generate_comparison(gen, node, "setl");
                    break;
                case TOKEN_LESS_EQUALS:
                    generate_comparison(gen, node, "setle");
                    break;
                case TOKEN_GREATER:
                    generate_comparison(gen, node, "setg");
                    break;
                case TOKEN_GREATER_EQUALS:
                    generate_comparison(gen, node, "setge");
                    break;
            }
            break;
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
        case NODE_ASSIGNMENT:
            // Handle NODE_ASSIGNMENT
            break;
        case NODE_RETURN:
            // Handle NODE_RETURN
            break;
        case NODE_IF:
            // Handle NODE_IF
            break;
        case NODE_WHILE:
            // Handle NODE_WHILE
            break;
        case NODE_COMPOUND_STATEMENT:
            // Handle NODE_COMPOUND_STATEMENT
            break;
        case NODE_VARIABLE_DECLARATION:
            // Handle NODE_VARIABLE_DECLARATION
            break;
        case NODE_FUNCTION_DECLARATION:
            // Handle NODE_FUNCTION_DECLARATION
            break;
        // Add default case to handle unexpected values
        default:
            // Handle unexpected node types
            break;
    }
}

static void generate_statement(CodeGenerator* gen, ASTNode* node) {
    switch (node->type) {
        case NODE_RETURN:
            generate_expression(gen, node->data.binary.left);
            emit(gen, "    movq %%rbp, %%rsp\n");
            emit(gen, "    popq %%rbp\n");
            emit(gen, "    ret\n");
            break;
            
        case NODE_IF: {
            int else_label = new_label(gen);
            int end_label = new_label(gen);
            
            // Generate condition
            generate_expression(gen, node->data.if_statement.condition);
            emit(gen, "    cmp $0, %%rax\n");
            emit(gen, "    je .L%d\n", else_label);
            
            // Generate if body
            generate_statement(gen, node->data.if_statement.if_body);
            emit(gen, "    jmp .L%d\n", end_label);
            
            // Generate else body
            emit(gen, ".L%d:\n", else_label);
            if (node->data.if_statement.else_body) {
                generate_statement(gen, node->data.if_statement.else_body);
            }
            
            emit(gen, ".L%d:\n", end_label);
            break;
        }
            
        case NODE_WHILE: {
            int start_label = new_label(gen);
            int end_label = new_label(gen);
            
            emit(gen, ".L%d:\n", start_label);
            generate_expression(gen, node->data.while_statement.condition);
            emit(gen, "    cmp $0, %%rax\n");
            emit(gen, "    je .L%d\n", end_label);
            
            generate_statement(gen, node->data.while_statement.body);
            emit(gen, "    jmp .L%d\n", start_label);
            emit(gen, ".L%d:\n", end_label);
            break;
        }
            
        case NODE_COMPOUND_STATEMENT:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                generate_statement(gen, node->data.block.statements[i]);
            }
            break;
            
        case NODE_VARIABLE_DECLARATION:
            if (node->data.variable.initializer) {
                generate_expression(gen, node->data.variable.initializer);
                emit(gen, "    movq %%rax, %d(%%rbp)\n",
                     get_variable_offset(gen, node->data.variable.name));
            }
            break;
            
        case NODE_ASSIGNMENT:
            generate_expression(gen, node->data.binary.right);
            emit(gen, "    movq %%rax, %d(%%rbp)\n",
                 get_variable_offset(gen, 
                                   node->data.binary.left->data.string_value));
            break;
    }
}

void generate_code(CodeGenerator* gen, ASTNode* node) {
    // Generate assembly header
    emit(gen, "    .global main\n");
    emit(gen, "    .text\n");
    
    // Generate each function
    for (int i = 0; i < node->data.block.statement_count; i++) {
        ASTNode* func = node->data.block.statements[i];
        if (func->type == NODE_FUNCTION_DECLARATION) {
            emit(gen, "%s:\n", func->data.function.name);
            
            // Function prologue
            emit(gen, "    pushq %%rbp\n");
            emit(gen, "    movq %%rsp, %%rbp\n");
            
            // Reserve stack space for local variables
            gen->stack_offset = 0;
            generate_statement(gen, func->data.function.body);
            
            if (gen->stack_offset > 0) {
                emit(gen, "    subq $%d, %%rsp\n", gen->stack_offset);
            }
        }
    }
}

void free_generator(CodeGenerator* gen) {
    for (int i = 0; i < gen->variables.count; i++) {
        free(gen->variables.names[i]);
    }
    free(gen->variables.names);
    free(gen->variables.offsets);
    fclose(gen->output);
    free(gen);
}

void generate_code_from_ir(CodeGenerator* gen, IRProgram* program) {
    // Generate assembly header
    emit(gen, "    .global main\n");
    emit(gen, "    .text\n");
    
    // Generate code for each instruction
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        
        switch (instr->op) {
            case IR_LABEL:
                emit(gen, "%s:\n", instr->label->name);
                if (instr->label->number == -1) {  // Function label
                    // Function prologue
                    emit(gen, "    pushq %%rbp\n");
                    emit(gen, "    movq %%rsp, %%rbp\n");
                }
                break;
                
            case IR_ADD:
                emit(gen, "    movq %s, %%rax\n", instr->src1);
                emit(gen, "    addq %s, %%rax\n", instr->src2);
                emit(gen, "    movq %%rax, %s\n", instr->dest);
                break;
                
            case IR_SUB:
                emit(gen, "    movq %s, %%rax\n", instr->src1);
                emit(gen, "    subq %s, %%rax\n", instr->src2);
                emit(gen, "    movq %%rax, %s\n", instr->dest);
                break;
                
            case IR_MUL:
                emit(gen, "    movq %s, %%rax\n", instr->src1);
                emit(gen, "    imulq %s, %%rax\n", instr->src2);
                emit(gen, "    movq %%rax, %s\n", instr->dest);
                break;
                
            case IR_DIV:
                emit(gen, "    movq %s, %%rax\n", instr->src1);
                emit(gen, "    cqto\n");
                emit(gen, "    idivq %s\n", instr->src2);
                emit(gen, "    movq %%rax, %s\n", instr->dest);
                break;
                
            case IR_ASSIGN:
                if (instr->src1) {
                    emit(gen, "    movq %s, %%rax\n", instr->src1);
                } else {
                    emit(gen, "    movq $%d, %%rax\n", instr->value);
                }
                emit(gen, "    movq %%rax, %s\n", instr->dest);
                break;
                
            case IR_JUMP:
                emit(gen, "    jmp %s\n", instr->label->name);
                break;
                
            case IR_JUMPZ:
                emit(gen, "    cmpq $0, %s\n", instr->src1);
                emit(gen, "    je %s\n", instr->label->name);
                break;
                
            case IR_JUMPNZ:
                emit(gen, "    cmpq $0, %s\n", instr->src1);
                emit(gen, "    jne %s\n", instr->label->name);
                break;
                
            case IR_CALL:
                // Handle function calls similar to before...
                break;
                
            case IR_RETURN:
                if (instr->src1) {
                    emit(gen, "    movq %s, %%rax\n", instr->src1);
                }
                emit(gen, "    movq %%rbp, %%rsp\n");
                emit(gen, "    popq %%rbp\n");
                emit(gen, "    ret\n");
                break;
        }
    }
}

void peephole_optimize(char* assembly) {
    (void)assembly; // Prevent unused parameter warning
    // Function implementation...
} 