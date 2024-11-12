#ifndef CODEGEN_H
#define CODEGEN_H

#include "compiler.h"

typedef struct {
    FILE* output;
    int label_count;
    int stack_offset;
    // Symbol table for variable tracking
    struct {
        char** names;
        int* offsets;
        int count;
    } variables;
} CodeGenerator;

CodeGenerator* create_generator(const char* output_filename);
void generate_code(CodeGenerator* gen, ASTNode* node);
void free_generator(CodeGenerator* gen);

#endif 