#include "compiler.h"
#include "codegen.h"
#include "ir.h"
#include "optimizer.h"
#include "semantic.h"

void print_phase_separator(const char* phase_name) {
    print_n_chars('=', 80);
    printf("\nPhase: %s\n", phase_name);
    print_n_chars('=', 80);
    printf("\n\n");
}

void print_source_code(const char* source) {
    printf("Source Code:\n");
    printf("------------\n");
    printf("%s\n", source);
}

void print_tokens(Lexer* lexer) {
    printf("Tokens:\n");
    printf("-------\n");
    Token* token;
    do {
        token = get_next_token(lexer);
        printf("%-15s | Value: '%s'\n", 
               get_token_name(token->type), 
               token->value ? token->value : "null");
        free(token->value);
        free(token);
    } while (token->type != TOKEN_EOF);
}

void print_ast_with_header(ASTNode* ast) {
    printf("Abstract Syntax Tree:\n");
    printf("--------------------\n");
    print_ast(ast, 0);
    printf("\n");
}

void print_symbol_table(SemanticAnalyzer* analyzer) {
    printf("Symbol Table:\n");
    printf("-------------\n");
    printf("%-20s | %-10s | %-10s | %s\n", 
           "Name", "Type", "Scope", "Category");
    printf("%s\n", "-");
    print_n_chars('-', 60);
    
    for (int i = 0; i < analyzer->table->count; i++) {
        Symbol* sym = &analyzer->table->symbols[i];
        printf("%-20s | %-10s | %-10d | %s\n",
               sym->name,
               sym->type,
               sym->scope_level,
               sym->is_function ? "Function" : "Variable");
    }
    printf("\n");
}

void print_ir_code(IRProgram* ir) {
    printf("Intermediate Representation:\n");
    printf("--------------------------\n");
    print_ir(ir);
    printf("\n");
}

void print_optimizations(OptFlags flags) {
    printf("Applied Optimizations:\n");
    printf("--------------------\n");
    printf("✓ Constant Folding:            %s\n", flags.constant_folding ? "Enabled" : "Disabled");
    printf("✓ Dead Code Elimination:       %s\n", flags.dead_code_elimination ? "Enabled" : "Disabled");
    printf("✓ Common Subexpression Elim:   %s\n", flags.common_subexpression ? "Enabled" : "Disabled");
    printf("✓ Loop Unrolling:             %s\n", flags.loop_unrolling ? "Enabled" : "Disabled");
    printf("✓ Strength Reduction:         %s\n", flags.strength_reduction ? "Enabled" : "Disabled");
    printf("✓ Tail Recursion Elimination: %s\n", flags.tail_recursion ? "Enabled" : "Disabled");
    printf("✓ Function Inlining:          %s\n", flags.inline_functions ? "Enabled" : "Disabled");
    printf("\n");
}

void print_assembly(const char* filename) {
    printf("Generated Assembly:\n");
    printf("------------------\n");
    FILE* asm_file = fopen(filename, "r");
    if (asm_file) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), asm_file)) {
            printf("%s", buffer);
        }
        fclose(asm_file);
    }
    printf("\n");
}

void print_n_chars(char c, int n) {
    for (int i = 0; i < n; i++) {
        putchar(c);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.c> <output.s>\n", argv[0]);
        return 1;
    }

    // Read source file
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* source = (char*)malloc(size + 1);
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);

    // Phase 1: Lexical Analysis
    print_phase_separator("1. Lexical Analysis");
    print_source_code(source);
    Lexer* lexer = create_lexer(source);
    char* source_copy = strdup(source);
    Lexer* parser_lexer = create_lexer(source_copy);
    print_tokens(lexer);

    // Phase 2: Syntax Analysis
    print_phase_separator("2. Syntax Analysis");
    Parser* parser = create_parser(parser_lexer);
    ASTNode* ast = parse(parser);
    print_ast_with_header(ast);

    // Phase 3: Semantic Analysis
    print_phase_separator("3. Semantic Analysis");
    SemanticAnalyzer* analyzer = create_analyzer();
    if (!analyze(analyzer, ast)) {
        fprintf(stderr, "Semantic error: %s\n", get_semantic_error(analyzer));
        // Cleanup and exit
        free_analyzer(analyzer);
        free_ast(ast);
        free_parser(parser);
        free_lexer(lexer);
        free_lexer(parser_lexer);
        free(source);
        free(source_copy);
        return 1;
    }
    print_symbol_table(analyzer);

    // Phase 4: Intermediate Code Generation
    print_phase_separator("4. Intermediate Code Generation");
    IRProgram* ir = create_ir_program();
    generate_ir(ir, ast);
    print_ir_code(ir);

    // Phase 5: Code Optimization
    print_phase_separator("5. Code Optimization");
    OptFlags opt_flags = {
        .constant_folding = true,
        .dead_code_elimination = true,
        .common_subexpression = true,
        .loop_unrolling = true,
        .strength_reduction = true,
        .tail_recursion = true,
        .inline_functions = true
    };
    
    print_optimizations(opt_flags);
    set_optimization_level(OPT_O2);
    optimize_program(ir, opt_flags);
    printf("Optimized IR:\n");
    print_ir_code(ir);

    // Phase 6: Code Generation
    print_phase_separator("6. Code Generation");
    CodeGenerator* gen = create_generator(argv[2]);
    generate_code_from_ir(gen, ir);
    print_assembly(argv[2]);

    printf("\nCompilation completed successfully!\n");
    printf("Output written to: %s\n", argv[2]);

    // Cleanup
    free_generator(gen);
    free_ast(ast);
    free_parser(parser);
    free_lexer(lexer);
    free_lexer(parser_lexer);
    free(source);
    free(source_copy);
    free_ir_program(ir);
    free_analyzer(analyzer);

    return 0;
} 