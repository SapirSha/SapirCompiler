#include "Lexer.h"


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "ArrayList.h"
#include "StringTrie.h"
#include "Queue.h"

#define DEFAULT_TOKEN_SIZE 32

Queue* tokens = NULL;

char* token_to_string(ArrayList* token) {
    static const char ZERO = '\0';
    arraylist_add(token, &ZERO);
    char* string = malloc(token->size * sizeof(char));
    for (int i = 0; i < token->size; i++) {
        string[i] = *(char*)token->array[i];
    }
    return string;
}

void add_token(ArrayList* token, Queue* tokens, Token_Types type) {
    char* str_token = token_to_string(token);
    queue_enqueue(tokens, &(Token){.lexeme = str_token, .type = type});
	arraylist_reset(token);
}

// Define states
typedef enum {
    START = 0,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    STRING_LITERAL,
    COMMENT,
    SEPARATOR,
    KEYWORD,
    ERROR,
    NUM_STATES,
};
typedef char State;

void handle_start(const char* input, int* index, ArrayList* token, State* next_state);
void handle_number(const char* input, int* index, ArrayList* token, State* next_state);
void handle_keyword(const char* input, int* index, ArrayList* token, State* next_state);
void handle_operator(const char* input, int* index, ArrayList* token, State* next_state);
void handle_string_literal(const char* input, int* index, ArrayList* token, State* next_state);
void handle_comment(const char* input, int* index, ArrayList* token, State* next_state);
void handle_separator(const char* input, int* index, ArrayList* token, State* next_state);
void handle_error(const char* input, int* index, ArrayList* token, State* next_state);
void handle_identifier(const char* input, int* index, ArrayList* token, State* next_state);


void(*states_functions[])(const char*, int*, ArrayList*, State*) = {
    [START] = handle_start,
    [NUMBER] = handle_number,
    [OPERATOR] = handle_operator,
    [STRING_LITERAL] = handle_string_literal,
    [COMMENT] = handle_comment,
    [SEPARATOR] = handle_separator,
    [KEYWORD] = handle_keyword,
    [ERROR] = handle_error,
    [IDENTIFIER] = handle_identifier,
};
void call_state_function(const char* input, int* index, ArrayList* token, State* next_state) {
    states_functions[*next_state](input, index, token, next_state);
}

static const Token_Types STATE_TO_TOKEN_CONVERTER[] = {
    [IDENTIFIER] = TOKEN_IDENTIFIER,
    [NUMBER] = TOKEN_NUMBER,
    [OPERATOR] = TOKEN_OPERATOR,
    [SEPARATOR] = TOKEN_SEPARATOR,
    [STRING_LITERAL] = TOKEN_STRING_LITERAL,
    [KEYWORD] = TOKEN_KEYWORD,
};

// Define character classes
typedef enum {
    CHAR_INVALID = 0,
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_OPERATOR,
    CHAR_WHITESPACE,
    CHAR_QUOTE, // For handling the double quote for string literals
    CHAR_COMMENT, // For handling comments
    CHAR_SEPARATOR,
    NUM_CHAR_CLASSES
};
typedef char CharClass;

// Lookup table for state transitions
static const State state_table[NUM_STATES][NUM_CHAR_CLASSES] = {
    // State: / Got:        CHAR_INVALID    CHAR_LETTER     CHAR_DIGIT      CHAR_OPERATOR    CHAR_WHITESPACE   CHAR_QUOTE          CHAR_COMMENT     CHAR_SEPARATOR
    /* START */           { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR},
    /* IDENTIFIER */      { ERROR,          IDENTIFIER,     IDENTIFIER,     OPERATOR,        START,            ERROR,              COMMENT,          SEPARATOR},
    /* NUMBER */          { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            ERROR,              COMMENT,          SEPARATOR},
    /* OPERATOR */        { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR},
    /* STRING_LITERAL */  { ERROR,          STRING_LITERAL, STRING_LITERAL, STRING_LITERAL,  STRING_LITERAL,   START,              STRING_LITERAL,   STRING_LITERAL},
    /* COMMENT */         { ERROR,          COMMENT,        COMMENT,        COMMENT,         COMMENT,          COMMENT,            START,            COMMENT},
    /* SEPARATOR */       { ERROR,          IDENTIFIER,     NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          STRING_LITERAL},
    /* KeyWord */         { ERROR,          KEYWORD,        KEYWORD,        OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR},
    /* ERROR */           { ERROR,          ERROR,          ERROR,          ERROR,           ERROR,            ERROR,              ERROR,            ERROR},
};


// Lookup table for all allowable characters
static const CharClass classifier_lookup[] = {
    // All other characters are invalid
    [0] = CHAR_INVALID,

    //NUMBERS
    ['1'] = CHAR_DIGIT,['2'] = CHAR_DIGIT,['3'] = CHAR_DIGIT,['4'] = CHAR_DIGIT,
    ['5'] = CHAR_DIGIT,['6'] = CHAR_DIGIT,['7'] = CHAR_DIGIT,['8'] = CHAR_DIGIT,
    ['9'] = CHAR_DIGIT,['0'] = CHAR_DIGIT,

    //LETTERS
    ['A'] = CHAR_LETTER,['B'] = CHAR_LETTER,['C'] = CHAR_LETTER,['D'] = CHAR_LETTER,
    ['E'] = CHAR_LETTER,['F'] = CHAR_LETTER,['G'] = CHAR_LETTER,['H'] = CHAR_LETTER,
    ['I'] = CHAR_LETTER,['J'] = CHAR_LETTER,['K'] = CHAR_LETTER,['L'] = CHAR_LETTER,
    ['M'] = CHAR_LETTER,['N'] = CHAR_LETTER,['O'] = CHAR_LETTER,['P'] = CHAR_LETTER,
    ['Q'] = CHAR_LETTER,['R'] = CHAR_LETTER,['S'] = CHAR_LETTER,['T'] = CHAR_LETTER,
    ['U'] = CHAR_LETTER,['V'] = CHAR_LETTER,['W'] = CHAR_LETTER,['X'] = CHAR_LETTER,
    ['Y'] = CHAR_LETTER,['Z'] = CHAR_LETTER,
    ['a'] = CHAR_LETTER,['b'] = CHAR_LETTER,['c'] = CHAR_LETTER,['d'] = CHAR_LETTER,
    ['e'] = CHAR_LETTER,['f'] = CHAR_LETTER,['g'] = CHAR_LETTER,['h'] = CHAR_LETTER,
    ['i'] = CHAR_LETTER,['j'] = CHAR_LETTER,['k'] = CHAR_LETTER,['l'] = CHAR_LETTER,
    ['m'] = CHAR_LETTER,['n'] = CHAR_LETTER,['o'] = CHAR_LETTER,['p'] = CHAR_LETTER,
    ['q'] = CHAR_LETTER,['r'] = CHAR_LETTER,['s'] = CHAR_LETTER,['t'] = CHAR_LETTER,
    ['u'] = CHAR_LETTER,['v'] = CHAR_LETTER,['w'] = CHAR_LETTER,['x'] = CHAR_LETTER,
    ['y'] = CHAR_LETTER,['z'] = CHAR_LETTER,
    ['_'] = CHAR_LETTER,

    //OPERATORS
    ['+'] = CHAR_OPERATOR,['-'] = CHAR_OPERATOR,['*'] = CHAR_OPERATOR,['/'] = CHAR_OPERATOR,
    ['='] = CHAR_OPERATOR,['>'] = CHAR_OPERATOR,['<'] = CHAR_OPERATOR,['%'] = CHAR_OPERATOR,
    ['&'] = CHAR_OPERATOR,['|'] = CHAR_OPERATOR,['^'] = CHAR_OPERATOR,['!'] = CHAR_OPERATOR,

    //SEPARATORS
    ['('] = CHAR_SEPARATOR,[')'] = CHAR_SEPARATOR,['{'] = CHAR_SEPARATOR,['}'] = CHAR_SEPARATOR,
    ['['] = CHAR_SEPARATOR,[']'] = CHAR_SEPARATOR,[';'] = CHAR_SEPARATOR,[','] = CHAR_SEPARATOR,


    //Spaces
    ['\n'] = CHAR_WHITESPACE,[' '] = CHAR_WHITESPACE,['\t'] = CHAR_WHITESPACE,

    //Comments
    ['#'] = CHAR_COMMENT,

    //Double quote for string literals
    ['"'] = CHAR_QUOTE,

    // All other characters are invalid
    [255] = CHAR_INVALID,
};

static const char* keywords_list[] = {
    "if", "while", "return", "for", "else", "int", "char",
	"string", "float", "double", "void", "bool", "true", "false",
	NULL
};

StringTrie* token_finder = NULL;
void init_finder() {
    if (token_finder != NULL) return;

    token_finder = stringin_init();
    stringin_insert_string(token_finder, "int", TOKEN_INT);
    stringin_insert_string(token_finder, "if", TOKEN_IF);
    stringin_insert_string(token_finder, "while", TOKEN_WHILE);
    stringin_insert_string(token_finder, "for", TOKEN_FOR);
	stringin_insert_string(token_finder, "else", TOKEN_ELSE);
	stringin_insert_string(token_finder, "char", TOKEN_CHAR);
	stringin_insert_string(token_finder, "string", TOKEN_STRING);
    stringin_insert_string(token_finder, "float", TOKEN_FLOAT);
	stringin_insert_string(token_finder, "double", TOKEN_DOUBLE);
	stringin_insert_string(token_finder, "void", TOKEN_VOID);
    stringin_insert_string(token_finder, "bool", TOKEN_BOOL);
    stringin_insert_string(token_finder, "true", TOKEN_TRUE);
	stringin_insert_string(token_finder, "false", TOKEN_FALSE);
    stringin_insert_string(token_finder, "continue", TOKEN_CONTINUE);
    stringin_insert_string(token_finder, "then", TOKEN_THEN);
    stringin_insert_string(token_finder, "do", TOKEN_DO);
    stringin_insert_string(token_finder, "until", TOKEN_UNTIL);
    stringin_insert_string(token_finder, "change", TOKEN_CHANGE);
    stringin_insert_string(token_finder, "print", TOKEN_PRINT);
    stringin_insert_string(token_finder, "get", TOKEN_GET);
    stringin_insert_string(token_finder, "gets", TOKEN_GETS);
    stringin_insert_string(token_finder, "function", TOKEN_FUNCTION);
    stringin_insert_string(token_finder, "returns", TOKEN_RETURNS);
    stringin_insert_string(token_finder, "return", TOKEN_RETURN);
    stringin_insert_string(token_finder, "nothing", TOKEN_NOTHING);
    stringin_insert_string(token_finder, "call", TOKEN_CALL);
    stringin_insert_string(token_finder, "with", TOKEN_WITH);
    stringin_insert_string(token_finder, "break", TOKEN_BREAK);

    




    stringin_insert_string(token_finder, "+", TOKEN_OPERATOR_PLUS);
    stringin_insert_string(token_finder, "-", TOKEN_OPERATOR_MINUS);
    stringin_insert_string(token_finder, "*", TOKEN_OPERATOR_MULTIPLY);
    stringin_insert_string(token_finder, "/", TOKEN_OPERATOR_DIVIDE);
    stringin_insert_string(token_finder, "=", TOKEN_OPERATOR_ASSIGN);
    stringin_insert_string(token_finder, "<", TOKEN_OPERATOR_LESS);
    stringin_insert_string(token_finder, ">", TOKEN_OPERATOR_GREATER);
    stringin_insert_string(token_finder, "%", TOKEN_OPERATOR_MODULO);
    stringin_insert_string(token_finder, "&", TOKEN_OPERATOR_AND);
    stringin_insert_string(token_finder, "|", TOKEN_OPERATOR_OR);
    stringin_insert_string(token_finder, "^", TOKEN_OPERATOR_XOR);
    stringin_insert_string(token_finder, "!", TOKEN_OPERATOR_NOT);

    stringin_insert_string(token_finder, "++", TOKEN_OPERATOR_INCREMENT);
    stringin_insert_string(token_finder, "--", TOKEN_OPERATOR_DECREMENT);
    stringin_insert_string(token_finder, "+=", TOKEN_OPERATOR_ADD_ASSIGN);
    stringin_insert_string(token_finder, "-=", TOKEN_OPERATOR_SUB_ASSIGN);
    stringin_insert_string(token_finder, "*=", TOKEN_OPERATOR_MUL_ASSIGN);
    stringin_insert_string(token_finder, "/=", TOKEN_OPERATOR_DIV_ASSIGN);
    stringin_insert_string(token_finder, "%=", TOKEN_OPERATOR_MOD_ASSIGN);
    stringin_insert_string(token_finder, "&=", TOKEN_OPERATOR_AND_ASSIGN);
    stringin_insert_string(token_finder, "|=", TOKEN_OPERATOR_OR_ASSIGN);
    stringin_insert_string(token_finder, "^=", TOKEN_OPERATOR_XOR_ASSIGN);
    stringin_insert_string(token_finder, "!=", TOKEN_OPERATOR_NOT_EQUAL);
    stringin_insert_string(token_finder, ">=", TOKEN_OPERATOR_GREATER_EQUAL);
    stringin_insert_string(token_finder, "<=", TOKEN_OPERATOR_LESS_EQUAL);
    stringin_insert_string(token_finder, "<<", TOKEN_OPERATOR_LEFT_SHIFT);
    stringin_insert_string(token_finder, ">>", TOKEN_OPERATOR_RIGHT_SHIFT);
    stringin_insert_string(token_finder, "&&", TOKEN_OPERATOR_ALSO);
    stringin_insert_string(token_finder, "||", TOKEN_OPERATOR_EITHER);
    stringin_insert_string(token_finder, "==", TOKEN_OPERATOR_EQUAL);



}

void handle_error(const char* input, int* index, ArrayList* token, State* next_state) {
    char* str_token = token_to_string(token);
    printf("--- Error: '%c' Cannot come after state: %d. for token: %s\n", input[*index], *next_state, str_token);
    arraylist_reset(token);
    *next_state = START;
    (*index)++;
}

void handle_keyword(const char* input, int* index, ArrayList* token, State* next_state) {
    StringTrie* pos = token_finder;
    char* clearance = pos->to_clear;

    while (*next_state == KEYWORD) {
        arraylist_add(token, &input[*index]);
        if (stringin_next(&pos, &clearance, input[*index]) == NOT_FOUND) {
            *next_state = IDENTIFIER;
            (*index)++;
            handle_identifier(input, index, token, next_state);
            return;
        }

        (*index)++;
        *next_state = state_table[KEYWORD][classifier_lookup[input[*index]]];
    }

    if (stringin_next(&pos, &clearance, '\0') == FOUND) {
		add_token(token, tokens, pos->is_end);
        *next_state = KEYWORD;
    }
    else {
        *next_state = IDENTIFIER;
        handle_identifier(input, index, token, next_state);
    }
}

void handle_operator(const char* input, int* index, ArrayList* token, State* next_state) {
    StringTrie* pos = token_finder;
    char* clearance = pos->to_clear;

    while (*next_state == OPERATOR) {
        arraylist_add(token, &input[*index]);
        if (stringin_next(&pos, &clearance, input[*index]) == NOT_FOUND) {
            add_token(token, tokens, pos->is_end);
            (*index)++;
            return;
        }
        (*index)++;
        *next_state = state_table[OPERATOR][classifier_lookup[input[*index]]];
    }
    add_token(token, tokens, pos->is_end);
}
// TO BE CHANGED LATER?
static const Token_Types SEPARATOR_TO_TOKEN_CONVERTER[] = {
    ['('] = TOKEN_LPAREN,[')'] = TOKEN_RPAREN,
    ['{'] = TOKEN_LBRACES,['}'] = TOKEN_RBRACES,
    ['['] = TOKEN_LBRACKETS,[']'] = TOKEN_RBRACKETS,
    [';'] = TOKEN_SEMICOLON,[','] = TOKEN_COMMA,

};
void handle_separator(const char* input, int* index, ArrayList* token, State* next_state) {
    arraylist_add(token, &input[*index]);
	add_token(token, tokens, SEPARATOR_TO_TOKEN_CONVERTER[input[*index]]);
    (*index)++;
}

void handle_number(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = NUMBER;
    while (current == NUMBER) {
        arraylist_add(token, &input[*index]);
        (*index)++;
        current = state_table[NUMBER][classifier_lookup[input[*index]]];
    }
    if (input[*index] == '.') {
        arraylist_add(token, &input[*index]);
        (*index)++;

        current = state_table[NUMBER][classifier_lookup[input[*index]]];

        if (current != NUMBER)
        {
            handle_error(input, index, token, &current);
            return;
        }

        while (current == NUMBER) {
            arraylist_add(token, &input[*index]);
            (*index)++;
            current = state_table[NUMBER][classifier_lookup[input[*index]]];
        }
        add_token(token, tokens, TOKEN_FLOAT_NUMBER);
    }
    else {
		add_token(token, tokens, TOKEN_NUMBER);
    }
}

void handle_string_literal(const char* input, int* index, ArrayList* token, State* next_state) {
    State current;
    (*index)++;
    while ((current = state_table[STRING_LITERAL][classifier_lookup[input[*index]]]) == STRING_LITERAL) {
        arraylist_add(token, &input[*index]);
        (*index)++;
    }
    if (input[*index] == '"') {
        add_token(token, tokens, TOKEN_STRING_LITERAL);
        (*index)++;
        *next_state = START;
    }
	else {
		printf("--- Error: Unclosed string literal ---\n");
		handle_error(input, index, token, &current); // unclosed string literal
	}
}

void handle_comment(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = COMMENT;
    while (current == COMMENT) {
        (*index)++;
        current = state_table[COMMENT][classifier_lookup[input[*index]]];
    }
    *next_state = START;
    (*index)++;
}

void handle_start(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = START;
    while (current == START) {
        if (input[*index] == '\n') {
            arraylist_add(token, &input[*index]);
            add_token(token, tokens, SEPARATOR_TO_TOKEN_CONVERTER[';']);
            (*index)++;
        }
        (*index)++;
        current = state_table[START][classifier_lookup[input[*index]]];
    }
}

void handle_identifier(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = state_table[IDENTIFIER][classifier_lookup[input[*index]]];
    while (current == IDENTIFIER) {
        arraylist_add(token, &input[*index]);
        (*index)++;
        current = state_table[IDENTIFIER][classifier_lookup[input[*index]]];
    }

	add_token(token, tokens, TOKEN_IDENTIFIER);
}

// FSM for tokenization
Queue* tokenize(const char* input) {
    State state = START, next_state;
    ArrayList* token = arraylist_init(sizeof(char), DEFAULT_TOKEN_SIZE);

    tokens = queue_init(sizeof(Token));

    init_finder();
    int i = 0;
    while (input[i] != '\0') {
        next_state = state_table[START][classifier_lookup[input[i]]];
        call_state_function(input, &i, token, &next_state);
        state = next_state;
    }

    queue_enqueue(tokens, &(Token){.type = TOKEN_EOF, .lexeme = "END"});
    queue_enqueue(tokens, &(Token){.type = TOKEN_EOF, .lexeme = "END"});

    arraylist_free(token);
    stringin_free(token_finder);

    return tokens;
}