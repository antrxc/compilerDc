#include "optimizer.h"
#include <stdbool.h>
#include "ir.h"

static OptLevel current_level = OPT_NONE;

// Helper function to check if an instruction is a computation
static bool is_computation(IRInstr* instr) {
    return instr->op == IR_ADD || instr->op == IR_SUB || 
           instr->op == IR_MUL || instr->op == IR_DIV;
}

// Common subexpression elimination
static void eliminate_common_subexpressions(IRProgram* program) {
    for (int i = 0; i < program->count; i++) {
        IRInstr* current = program->instructions[i];
        if (!is_computation(current)) continue;
        
        // Look for identical computations
        for (int j = i + 1; j < program->count; j++) {
            IRInstr* next = program->instructions[j];
            if (!is_computation(next)) continue;
            
            // Check if operations and operands match
            if (next->op == current->op &&
                strcmp(next->src1, current->src1) == 0 &&
                strcmp(next->src2, current->src2) == 0) {
                // Replace computation with assignment
                next->op = IR_ASSIGN;
                free(next->src2);
                next->src2 = NULL;
                free(next->src1);
                next->src1 = strdup(current->dest);
            }
        }
    }
}

// Strength reduction (replace expensive operations with cheaper ones)
static void reduce_strength(IRProgram* program) {
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        
        // Replace multiplication by 2 with addition
        if (instr->op == IR_MUL) {
            int value = atoi(instr->src2);
            if (value == 2) {
                instr->op = IR_ADD;
                free(instr->src2);
                instr->src2 = strdup(instr->src1);
            }
        }
        
        // Replace division by 2 with right shift
        if (instr->op == IR_DIV) {
            int value = atoi(instr->src2);
            if (value == 2) {
                instr->op = IR_SHR;  // Add IR_SHR to IROpcode enum
                free(instr->src2);
                instr->src2 = strdup("1");
            }
        }
    }
}

// Loop unrolling
static void unroll_loops(IRProgram* program) {
    for (int i = 0; i < program->block_count; i++) {
        BasicBlock* block = program->blocks[i];
        IRInstr* last_instr = program->instructions[block->end];
        
        // Check if this is a loop block
        if (last_instr->op == IR_JUMP) {
            // Find loop condition
            for (int j = 0; j < program->block_count; j++) {
                BasicBlock* target = program->blocks[j];
                if (target->start < block->start && 
                    strcmp(last_instr->label->name, 
                           program->instructions[target->start]->label->name) == 0) {
                    // This is a backward jump - likely a loop
                    // Unroll the loop if it's small enough
                    int loop_size = block->end - block->start + 1;
                    if (loop_size < 10) {  // Only unroll small loops
                        // Duplicate the loop body
                        for (int k = block->start; k <= block->end - 1; k++) {
                            IRInstr* instr = program->instructions[k];
                            // Create a copy of the instruction
                            IRInstr* copy = malloc(sizeof(IRInstr));
                            memcpy(copy, instr, sizeof(IRInstr));
                            if (copy->dest) copy->dest = strdup(instr->dest);
                            if (copy->src1) copy->src1 = strdup(instr->src1);
                            if (copy->src2) copy->src2 = strdup(instr->src2);
                            // Insert the copy
                            // Note: Need to implement instruction insertion
                        }
                    }
                    break;
                }
            }
        }
    }
}

// Tail recursion elimination
static void eliminate_tail_recursion(IRProgram* program) {
    char* current_function = NULL;
    
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        
        if (instr->op == IR_LABEL && instr->label->number == -1) {
            // This is a function label
            current_function = strdup(instr->label->name);
        }
        
        if (instr->op == IR_CALL && 
            strcmp(instr->src1, current_function) == 0) {
            // Found a recursive call
            // Check if it's followed by a return
            if (i + 1 < program->count && 
                program->instructions[i + 1]->op == IR_RETURN) {
                // This is tail recursion - replace with jump
                instr->op = IR_JUMP;
                IRLabel* label = malloc(sizeof(IRLabel));
                label->name = strdup(current_function);
                label->number = -1;
                instr->label = label;
                
                // Remove the return instruction
                free_instruction(program->instructions[i + 1]);
                // Shift remaining instructions
                for (int j = i + 1; j < program->count - 1; j++) {
                    program->instructions[j] = program->instructions[j + 1];
                }
                program->count--;
            }
        }
    }
    
    free(current_function);
}

// Function inlining
static void inline_functions(IRProgram* program) {
    // First pass: collect small functions
    struct {
        char* name;
        int start;
        int end;
        int instruction_count;
    } functions[100];
    int function_count = 0;
    
    int current_start = 0;
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        if (instr->op == IR_LABEL && instr->label->number == -1) {
            current_start = i;
        } else if (instr->op == IR_RETURN) {
            int size = i - current_start + 1;
            if (size < 20) {  // Only inline small functions
                functions[function_count].name = 
                    strdup(program->instructions[current_start]->label->name);
                functions[function_count].start = current_start;
                functions[function_count].end = i;
                functions[function_count].instruction_count = size;
                function_count++;
            }
        }
    }
    
    // Second pass: replace calls with inlined code
    for (int i = 0; i < program->count; i++) {
        IRInstr* instr = program->instructions[i];
        if (instr->op == IR_CALL) {
            // Check if this function should be inlined
            for (int j = 0; j < function_count; j++) {
                if (strcmp(instr->src1, functions[j].name) == 0) {
                    // Inline the function
                    // Note: Need to implement instruction insertion and
                    // handle parameter passing
                }
            }
        }
    }
}

void set_optimization_level(OptLevel level) {
    current_level = level;
}

void optimize_program(IRProgram* program, OptFlags flags) {
    if (flags.constant_folding) {
        constant_folding(program);
    }
    
    if (flags.dead_code_elimination) {
        dead_code_elimination(program);
    }
    
    if (flags.common_subexpression) {
        eliminate_common_subexpressions(program);
    }
    
    if (flags.strength_reduction) {
        reduce_strength(program);
    }
    
    if (flags.loop_unrolling) {
        unroll_loops(program);
    }
    
    if (flags.tail_recursion) {
        eliminate_tail_recursion(program);
    }
    
    if (flags.inline_functions) {
        inline_functions(program);
    }
}

// Peephole optimization for assembly code
void peephole_optimize(char* assembly) {
    (void)assembly; // Prevent unused parameter warning
    // Common x86_64 peephole optimizations
    
    // 1. Remove redundant moves
    // mov %rax, %rbx
    // mov %rbx, %rax  -> remove second move
    
    // 2. Simplify arithmetic
    // add $0, %rax    -> remove
    // mul $1, %rax    -> remove
    // sub $0, %rax    -> remove
    
    // 3. Optimize jumps
    // jmp L1
    // L1:             -> remove jump
    
    // 4. Merge load/store
    // mov %rax, (%rsp)
    // mov (%rsp), %rax  -> remove second move
    
    // Implementation would involve pattern matching on assembly strings
} 