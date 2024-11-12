#include "ir.h"
#include <stdbool.h>

static int evaluate_constant_expr(IROpcode op, int left, int right) {
    switch (op) {
        case IR_ADD: return left + right;
        case IR_SUB: return left - right;
        case IR_MUL: return left * right;
        case IR_DIV: return left / right;
        default: return 0;
    }
}

static bool is_constant(IRInstr* instr) {
    return instr->op == IR_ASSIGN && instr->src1 == NULL && instr->src2 == NULL;
}

static int get_constant_value(IRProgram* program, const char* temp) {
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        if (instr->dest && strcmp(instr->dest, temp) == 0 && is_constant(instr)) {
            return instr->value;
        }
    }
    return 0;
}

void constant_folding(IRProgram* program) {
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < program->count; i++) {
            IRInstr* instr = program->instructions[i];
            
            // Look for arithmetic operations with constant operands
            if ((instr->op == IR_ADD || instr->op == IR_SUB || 
                 instr->op == IR_MUL || instr->op == IR_DIV) &&
                instr->src1 && instr->src2) {
                
                int left_val, right_val;
                bool left_const = false, right_const = false;
                
                // Check if operands are constants
                for (int j = 0; j < i; j++) {
                    IRInstr* prev = program->instructions[j];
                    if (prev->dest) {
                        if (strcmp(prev->dest, instr->src1) == 0 && is_constant(prev)) {
                            left_val = prev->value;
                            left_const = true;
                        }
                        if (strcmp(prev->dest, instr->src2) == 0 && is_constant(prev)) {
                            right_val = prev->value;
                            right_const = true;
                        }
                    }
                }
                
                // If both operands are constants, fold them
                if (left_const && right_const) {
                    int result = evaluate_constant_expr(instr->op, left_val, right_val);
                    instr->op = IR_ASSIGN;
                    free(instr->src1);
                    free(instr->src2);
                    instr->src1 = NULL;
                    instr->src2 = NULL;
                    instr->value = result;
                    changed = true;
                }
            }
        }
    } while (changed);
}

static void find_basic_blocks(IRProgram* program) {
    // First pass: count basic blocks
    int block_count = 1;  // Start block
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        if (instr->op == IR_LABEL || 
            instr->op == IR_JUMP || 
            instr->op == IR_JUMPZ || 
            instr->op == IR_JUMPNZ) {
            block_count++;
        }
    }
    
    // Allocate blocks
    program->blocks = malloc(sizeof(BasicBlock*) * block_count);
    program->block_count = 0;
    
    // Second pass: create basic blocks
    int current_start = 0;
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        if (instr->op == IR_LABEL || i == 0) {
            BasicBlock* block = malloc(sizeof(BasicBlock));
            block->start = current_start;
            block->end = i - 1;
            block->successors = malloc(sizeof(BasicBlock*) * 2);  // Max 2 successors
            block->successor_count = 0;
            block->is_reachable = false;
            
            if (program->block_count > 0) {
                program->blocks[program->block_count - 1]->end = i - 1;
            }
            
            program->blocks[program->block_count++] = block;
            current_start = i;
        }
    }
    
    // Set end of last block
    if (program->block_count > 0) {
        program->blocks[program->block_count - 1]->end = program->count - 1;
    }
}

void dead_code_elimination(IRProgram* program) {
    find_basic_blocks(program);
    
    // Mark reachable blocks starting from entry
    program->blocks[0]->is_reachable = true;
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < program->block_count; i++) {
            BasicBlock* block = program->blocks[i];
            if (!block->is_reachable) continue;
            
            // Find successors
            IRInstr* last_instr = program->instructions[block->end];
            if (last_instr->op == IR_JUMP || 
                last_instr->op == IR_JUMPZ || 
                last_instr->op == IR_JUMPNZ) {
                
                // Find target block
                for (int j = 0; j < program->block_count; j++) {
                    BasicBlock* target = program->blocks[j];
                    IRInstr* first_instr = program->instructions[target->start];
                    if (first_instr->op == IR_LABEL && 
                        strcmp(first_instr->label->name, last_instr->label->name) == 0) {
                        if (!target->is_reachable) {
                            target->is_reachable = true;
                            changed = true;
                        }
                        break;
                    }
                }
            }
            
            // Fall-through case
            if (i + 1 < program->block_count && !program->blocks[i + 1]->is_reachable) {
                program->blocks[i + 1]->is_reachable = true;
                changed = true;
            }
        }
    } while (changed);
    
    // Remove unreachable blocks
    int write = 0;
    for (int read = 0; read < program->count; read++) {
        IRInstr* instr = program->instructions[read];
        bool keep = false;
        
        // Check if instruction is in a reachable block
        for (int i = 0; i < program->block_count; i++) {
            BasicBlock* block = program->blocks[i];
            if (block->is_reachable && 
                read >= block->start && 
                read <= block->end) {
                keep = true;
                break;
            }
        }
        
        if (keep) {
            program->instructions[write++] = instr;
        } else {
            free(instr->dest);
            free(instr->src1);
            free(instr->src2);
            if (instr->label) {
                free(instr->label->name);
                free(instr->label);
            }
            free(instr);
        }
    }
    program->count = write;
}

void optimize_ir(IRProgram* program) {
    constant_folding(program);
    dead_code_elimination(program);
} 