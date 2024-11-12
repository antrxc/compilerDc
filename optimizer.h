#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "ir.h"
#include <stdbool.h>

// Optimization levels
typedef enum {
    OPT_NONE = 0,
    OPT_O1,    // Basic optimizations
    OPT_O2,    // Moderate optimizations
    OPT_O3     // Aggressive optimizations
} OptLevel;

// Optimization flags
typedef struct {
    bool constant_folding;
    bool dead_code_elimination;
    bool common_subexpression;
    bool loop_unrolling;
    bool strength_reduction;
    bool tail_recursion;
    bool inline_functions;
} OptFlags;

// Function declarations
void set_optimization_level(OptLevel level);
void optimize_program(IRProgram* program, OptFlags flags);
void peephole_optimize(char* assembly_code);

#endif 