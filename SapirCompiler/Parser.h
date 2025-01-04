// Parser.h
#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"

// AST node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_BLOCK,
    NODE_VARIABLE_DECL,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_IDENTIFIER,
    NODE_NUMBER,
    NODE_STRING
} NodeType;

// AST node structure
typedef struct ASTNode {
    NodeType type;
    union {
        // For binary expressions
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char* operator;
        } binary;

        // For if statements
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;

        // For while loops
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;

        // For variable declarations
        struct {
            char* type;
            char* name;
            struct ASTNode* initializer;
        } var_decl;

        // For literals
        union {
            int number_value;
            char* string_value;
            char* identifier;
        } literal;
    } data;
} ASTNode;

// Parser structure
typedef struct {
    Tokens* tokens;
    Token* current_token;
} Parser;

// Function declarations
Parser* parser_init(Tokens* tokens);
ASTNode* parse(Parser* parser);
void parser_free(Parser* parser);
void ast_free(ASTNode* node);

#endif // PARSER_H