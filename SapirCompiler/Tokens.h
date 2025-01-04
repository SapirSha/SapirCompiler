#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_UNKNOWN = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FLOAT_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_STRING_LITERAL,
    TOKEN_SEPARATOR,

    TOKEN_OPERATOR_PLUS,        // +
    TOKEN_OPERATOR_MINUS,       // -
    TOKEN_OPERATOR_MULTIPLY,    // *
    TOKEN_OPERATOR_DIVIDE,      // /
    TOKEN_OPERATOR_ASSIGN,      // =
    TOKEN_OPERATOR_GREATER,     // >
    TOKEN_OPERATOR_LESS,        // <
    TOKEN_OPERATOR_MODULO,      // %
    TOKEN_OPERATOR_AND,         // &
    TOKEN_OPERATOR_OR,          // |
    TOKEN_OPERATOR_XOR,         // ^
    TOKEN_OPERATOR_NOT,         // !
    TOKEN_OPERATOR_INCREMENT,   // ++
    TOKEN_OPERATOR_DECREMENT,   // --
    TOKEN_OPERATOR_ADD_ASSIGN,  // +=
    TOKEN_OPERATOR_SUB_ASSIGN,  // -=
    TOKEN_OPERATOR_MUL_ASSIGN,  // *=
    TOKEN_OPERATOR_DIV_ASSIGN,  // /=
    TOKEN_OPERATOR_MOD_ASSIGN,  // %=
    TOKEN_OPERATOR_AND_ASSIGN,  // &=
    TOKEN_OPERATOR_OR_ASSIGN,   // |=
    TOKEN_OPERATOR_XOR_ASSIGN,  // ^=
    TOKEN_OPERATOR_LEFT_SHIFT,  // <<
    TOKEN_OPERATOR_RIGHT_SHIFT, // >>
    TOKEN_OPERATOR_EQUAL,       // ==
    TOKEN_OPERATOR_NOT_EQUAL,   // !=
    TOKEN_OPERATOR_GREATER_EQUAL, // >=
    TOKEN_OPERATOR_LESS_EQUAL,  // <=
    TOKEN_OPERATOR_ALSO,        // &&
    TOKEN_OPERATOR_EITHER,      // ||

    TOKEN_IF,
	TOKEN_WHILE,
	TOKEN_RETURN,
	TOKEN_FOR,
	TOKEN_ELSE,
	TOKEN_INT,
    TOKEN_STRING,
	TOKEN_CHAR,
	TOKEN_FLOAT,
	TOKEN_DOUBLE,
	TOKEN_VOID,
	TOKEN_BOOL,
	TOKEN_TRUE,
	TOKEN_FALSE,

    TOKEN_LPAREN, TOKEN_RPAREN, 
    TOKEN_LBRACES, TOKEN_RBRACES,
    TOKEN_LBRACKETS, TOKEN_RBRACKETS,
    TOKEN_SEMICOLON, 
    TOKEN_COMMA,

    TOKEN_EOF,
}Token_Types;

typedef struct Tokens{
    char* lexeme;
    Token_Types type;
    struct Tokens* next;
} Tokens;

typedef struct TokensQueue {
    Tokens* head;
    Tokens* tail;
} TokensQueue;


// Initialize the queue
TokensQueue* tokens_init();

// Enqueue a new token
void tokens_enqueue(TokensQueue* queue, char* lexeme, int type);

// Dequeue a token (removes and returns the front token)
Tokens* tokens_dequeue(TokensQueue* queue);

// Print all tokens in the queue
void tokens_print(TokensQueue* queue);

// Free the memory used by the queue
void tokens_free(TokensQueue* queue);

#endif