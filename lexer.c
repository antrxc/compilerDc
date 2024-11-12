#include "compiler.h"

Lexer* create_lexer(char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->length = strlen(source);
    return lexer;
}

static char peek(Lexer* lexer) {
    if (lexer->position >= lexer->length) {
        return '\0';
    }
    return lexer->source[lexer->position];
}

static char advance(Lexer* lexer) {
    if (lexer->position >= lexer->length) {
        return '\0';
    }
    return lexer->source[lexer->position++];
}

static void skip_whitespace(Lexer* lexer) {
    while (isspace(peek(lexer))) {
        advance(lexer);
    }
}

Token* get_next_token(Lexer* lexer) {
    skip_whitespace(lexer);

    Token* token = (Token*)malloc(sizeof(Token));
    char current = peek(lexer);

    if (current == '\0') {
        token->type = TOKEN_EOF;
        token->value = NULL;
        return token;
    }

    // Handle identifiers and keywords
    if (isalpha(current)) {
        int start = lexer->position;
        while (isalnum(peek(lexer))) {
            advance(lexer);
        }
        int length = lexer->position - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, &lexer->source[start], length);
        value[length] = '\0';

        // Check if it's a keyword
        if (strcmp(value, "int") == 0 || 
            strcmp(value, "return") == 0 || 
            strcmp(value, "if") == 0 || 
            strcmp(value, "else") == 0) {
            token->type = TOKEN_KEYWORD;
        } else {
            token->type = TOKEN_IDENTIFIER;
        }
        token->value = value;
        return token;
    }

    // Handle numbers
    if (isdigit(current)) {
        int start = lexer->position;
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        int length = lexer->position - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, &lexer->source[start], length);
        value[length] = '\0';
        token->type = TOKEN_NUMBER;
        token->value = value;
        return token;
    }

    // Handle multi-character operators and keywords
    if (isalpha(current)) {
        int start = lexer->position;
        while (isalnum(peek(lexer))) {
            advance(lexer);
        }
        int length = lexer->position - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, &lexer->source[start], length);
        value[length] = '\0';

        // Check keywords
        if (strcmp(value, "int") == 0) token->type = TOKEN_KEYWORD;
        else if (strcmp(value, "return") == 0) token->type = TOKEN_RETURN;
        else if (strcmp(value, "if") == 0) token->type = TOKEN_IF;
        else if (strcmp(value, "else") == 0) token->type = TOKEN_ELSE;
        else if (strcmp(value, "while") == 0) token->type = TOKEN_WHILE;
        else token->type = TOKEN_IDENTIFIER;
        
        token->value = value;
        return token;
    }

    // Handle two-character operators
    if (current == '=') {
        if (peek(lexer) == '=') {
            advance(lexer); // consume first '='
            advance(lexer); // consume second '='
            token->type = TOKEN_EQUALS;
            token->value = strdup("==");
            return token;
        }
    } else if (current == '!' && peek(lexer) == '=') {
        advance(lexer); // consume '!'
        advance(lexer); // consume '='
        token->type = TOKEN_NOT_EQUALS;
        token->value = strdup("!=");
        return token;
    } else if (current == '<' && peek(lexer) == '=') {
        advance(lexer);
        advance(lexer);
        token->type = TOKEN_LESS_EQUALS;
        token->value = strdup("<=");
        return token;
    } else if (current == '>' && peek(lexer) == '=') {
        advance(lexer);
        advance(lexer);
        token->type = TOKEN_GREATER_EQUALS;
        token->value = strdup(">=");
        return token;
    }

    // Handle operators
    token->value = (char*)malloc(2);
    token->value[0] = advance(lexer);
    token->value[1] = '\0';

    switch (token->value[0]) {
        case '+': token->type = TOKEN_PLUS; break;
        case '-': token->type = TOKEN_MINUS; break;
        case '*': token->type = TOKEN_MULTIPLY; break;
        case '/': token->type = TOKEN_DIVIDE; break;
        case '(': token->type = TOKEN_LPAREN; break;
        case ')': token->type = TOKEN_RPAREN; break;
        case ';': token->type = TOKEN_SEMICOLON; break;
        case '=': token->type = TOKEN_ASSIGN; break;
        case '{': token->type = TOKEN_LBRACE; break;
        case '}': token->type = TOKEN_RBRACE; break;
        case ',': token->type = TOKEN_COMMA; break;
        case '<': token->type = TOKEN_LESS; break;
        case '>': token->type = TOKEN_GREATER; break;
        default:
            fprintf(stderr, "Unknown token: %c\n", token->value[0]);
            exit(1);
    }

    return token;
}

void free_lexer(Lexer* lexer) {
    free(lexer);
} 