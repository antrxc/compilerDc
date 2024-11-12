#ifndef IR_H
#define IR_H

#include "compiler.h"
#include <stdbool.h>


typedef enum {
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_ASSIGN,
    IR_LABEL,
    IR_JUMP,
    IR_JUMPZ,    // Jump if zero
    IR_JUMPNZ,   // Jump if not zero
    IR_CALL,
    IR_RETURN,
    IR_PARAM,    // Function parameter
    IR_ARG,      // Function argument
    IR_COMPARE,
    IR_LOAD,
    IR_STORE,
    IR_SHR
} IROpcode;

typedef struct {
    char* name;
    int number;
} IRLabel;

typedef struct IRInstr {
    IROpcode op;
    char* dest;     // Destination operand
    char* src1;     // Source operand 1
    char* src2;     // Source operand 2
    IRLabel* label; // For jumps and labels
    int value;      // For immediate values
} IRInstr;
// Basic block structure for optimization
typedef struct BasicBlock {
    int start;              // Start instruction index
    int end;                // End instruction index
    struct BasicBlock** successors;  // Next basic blocks
    int successor_count;
    bool is_reachable;      // For dead code elimination
    IRInstr* instructions;  // Assuming IRInstr is defined elsewhere
    int instruction_count;
} BasicBlock;
typedef struct {
    IRInstr** instructions;
    int count;
    int capacity;
    int temp_count;     // Counter for temporary variables
    int label_count;    // Counter for labels
    BasicBlock** blocks;    // Array of basic blocks
    int block_count;
} IRProgram;

IRProgram* create_ir_program(void);
void generate_ir(IRProgram* program, ASTNode* ast);
char* new_temp(IRProgram* program);
IRLabel* new_label(IRProgram* program);
void add_instruction(IRProgram* program, IRInstr* instr);
void print_ir(IRProgram* program);
void free_ir_program(IRProgram* program);

// Optimization functions
void optimize_ir(IRProgram* program);
void constant_folding(IRProgram* program);
void dead_code_elimination(IRProgram* program);
void merge_basic_blocks(IRProgram* program);

void free_instruction(IRInstr* instr);



#endif 