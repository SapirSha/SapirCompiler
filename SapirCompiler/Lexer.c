#include "Lexer.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "HashMap.h"
#include "ArrayList.h"
#include "StringIn.h"

#define DEFAULT_TOKEN_SIZE 32

TokensQueue* tokens = NULL;

void add_token(ArrayList* token, TokensQueue* tokens, Token_Types type) {
	arraylist_add(token, '\0');
	char* str_token = token->array;
	tokens_enqueue(tokens, str_token, type);
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
    /* COMMENT */         { ERROR,          COMMENT,        COMMENT,        COMMENT,         COMMENT,          COMMENT,            START,            SEPARATOR},
    /* SEPARATOR */       { ERROR,          IDENTIFIER,     NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR},
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

StringIn* keywords_finder = NULL;
void init_keywords() {
    if (keywords_finder != NULL) return;

    keywords_finder = stringin_init();
    stringin_insert_string(keywords_finder, "int", TOKEN_INT);
    stringin_insert_string(keywords_finder, "if", TOKEN_IF);
    stringin_insert_string(keywords_finder, "while", TOKEN_WHILE);
    stringin_insert_string(keywords_finder, "for", TOKEN_FOR);
	stringin_insert_string(keywords_finder, "else", TOKEN_ELSE);
	stringin_insert_string(keywords_finder, "char", TOKEN_CHAR);
	stringin_insert_string(keywords_finder, "string", TOKEN_STRING);
    stringin_insert_string(keywords_finder, "float", TOKEN_FLOAT);
	stringin_insert_string(keywords_finder, "double", TOKEN_DOUBLE);
	stringin_insert_string(keywords_finder, "void", TOKEN_VOID);
    stringin_insert_string(keywords_finder, "bool", TOKEN_BOOL);
    stringin_insert_string(keywords_finder, "true", TOKEN_TRUE);
	stringin_insert_string(keywords_finder, "false", TOKEN_FALSE);
}

void handle_error(const char* input, int* index, ArrayList* token, State* next_state) {
    arraylist_add(token, '\0');
    printf("--- Error: '%c' Cannot come after state: %d. for token: %s\n", input[*index], *next_state, token->array);
    arraylist_reset(token);
    *next_state = START;
    (*index)++;
}

void handle_keyword(const char* input, int* index, ArrayList* token, State* next_state) {
    StringIn* pos = keywords_finder;
    char* clearance = pos->to_clear;

    while (*next_state == KEYWORD) {
        arraylist_add(token, input[*index]);
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
        (*index)++;
        handle_identifier(input, index, token, next_state);
    }
}

typedef enum {
    OPERATOR_START = 0,
    OPERATOR_PLUS,        // +
    OPERATOR_MINUS,       // -
    OPERATOR_MULTIPLY,    // *
    OPERATOR_DIVIDE,      // /
    OPERATOR_ASSIGN,      // =
    OPERATOR_GREATER,     // >
    OPERATOR_LESS,        // <
    OPERATOR_MODULO,      // %
    OPERATOR_AND,         // &
    OPERATOR_OR,          // |
    OPERATOR_XOR,         // ^
    OPERATOR_NOT,         // !
    NUM_OPERATOR_STATES,  // NUM OF OPERATOR STATES
    OPERATOR_ERROR,       // ERROR STATE

    OPERATOR_INCREMENT,   // ++
    OPERATOR_DECREMENT,   // --
    OPERATOR_ADD_ASSIGN,  // +=
    OPERATOR_SUB_ASSIGN,  // -=
    OPERATOR_MUL_ASSIGN,  // *=
    OPERATOR_DIV_ASSIGN,  // /=
    OPERATOR_MOD_ASSIGN,  // %=
    OPERATOR_AND_ASSIGN,  // &=
    OPERATOR_OR_ASSIGN,   // |=
    OPERATOR_XOR_ASSIGN,  // ^=
    OPERATOR_LEFT_SHIFT,  // <<
    OPERATOR_RIGHT_SHIFT, // >>
    // OPERATOR_LEFT_SHIFT_ASSIGN,  // <<=
    // OPERATOR_LEFT_RIGHT_ASSIGN,  // >>=
    OPERATOR_EQUAL,       // ==
    OPERATOR_NOT_EQUAL,   // !=
    OPERATOR_GREATER_EQUAL, // >=
    OPERATOR_LESS_EQUAL,  // <=
    OPERATOR_ALSO,        // &&
    OPERATOR_EITHER,      // ||
};
typedef char OperatorState;

static const OperatorState operator_lookup[] = {
    ['+'] = OPERATOR_PLUS,['-'] = OPERATOR_MINUS,['*'] = OPERATOR_MULTIPLY,['/'] = OPERATOR_DIVIDE,
    ['='] = OPERATOR_ASSIGN,['>'] = OPERATOR_GREATER,['<'] = OPERATOR_LESS,['%'] = OPERATOR_MODULO,
    ['&'] = OPERATOR_AND,['|'] = OPERATOR_OR,['^'] = OPERATOR_XOR,['!'] = OPERATOR_NOT,
}; int max_oper_ascii_index = sizeof(operator_lookup) / sizeof(char);


static const OperatorState operator_state_table[NUM_OPERATOR_STATES][NUM_OPERATOR_STATES] = {
    // State: / Got:       +  PLUS               - MINUS               * MULTIPLY           / DIVIDE               = EQUAL                > GREATER              < LESS               % MODULO           & AND               | OR                ^ XOR           ! NOT       
    /* Start */         {OPERATOR_ERROR, OPERATOR_PLUS,       OPERATOR_MINUS,       OPERATOR_MULTIPLY,   OPERATOR_DIVIDE,       OPERATOR_ASSIGN,         OPERATOR_GREATER,     OPERATOR_LESS,      OPERATOR_MODULO,   OPERATOR_AND,      OPERATOR_OR,        OPERATOR_XOR,     OPERATOR_NOT  },  // Start state
    /* PLUS */          {OPERATOR_ERROR, OPERATOR_INCREMENT,  OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_ADD_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // PLUS state
    /* MINUS */         {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_DECREMENT,   OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_SUB_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // MINUS state
    /* MULTIPLY */      {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_MUL_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // MULTIPLY state
    /* DIVIDE */        {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_DIV_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // DIVIDE state
    /* ASSIGN */        {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_EQUAL,          OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // ASSIGN state
    /* GREATER */       {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_GREATER_EQUAL,  OPERATOR_RIGHT_SHIFT, OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // GREATER state
    /* LESS */          {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_LESS_EQUAL,     OPERATOR_ERROR,       OPERATOR_LEFT_SHIFT,OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // LESS state
    /* MODULO */        {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_MOD_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // MODULO state
    /* AND */           {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_AND_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ALSO,     OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // AND state
    /* OR */            {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_OR_ASSIGN,      OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_EITHER,    OPERATOR_ERROR,   OPERATOR_ERROR},  // OR state
    /* XOR */           {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_XOR_ASSIGN,     OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // XOR state
    /* NOT */           {OPERATOR_ERROR, OPERATOR_ERROR,      OPERATOR_ERROR,       OPERATOR_ERROR,      OPERATOR_ERROR,        OPERATOR_NOT_EQUAL,      OPERATOR_ERROR,       OPERATOR_ERROR,     OPERATOR_ERROR,    OPERATOR_ERROR,    OPERATOR_ERROR,     OPERATOR_ERROR,   OPERATOR_ERROR},  // NOT state
};
static const Token_Types STATE_OPERATOR_TO_TOKEN_CONVERTER[] = {
    [OPERATOR_PLUS] = TOKEN_OPERATOR_PLUS,        // +
    [OPERATOR_MINUS] = TOKEN_OPERATOR_MINUS,       // -
    [OPERATOR_MULTIPLY] = TOKEN_OPERATOR_MULTIPLY,    // *
    [OPERATOR_DIVIDE] = TOKEN_OPERATOR_DIVIDE,      // /
    [OPERATOR_ASSIGN] = TOKEN_OPERATOR_ASSIGN,      // =
    [OPERATOR_GREATER] = TOKEN_OPERATOR_GREATER,     // >
    [OPERATOR_LESS] = TOKEN_OPERATOR_LESS,        // <
    [OPERATOR_MODULO] = TOKEN_OPERATOR_MODULO,      // %
    [OPERATOR_AND] = TOKEN_OPERATOR_AND,         // &
    [OPERATOR_OR] = TOKEN_OPERATOR_OR,          // |
    [OPERATOR_XOR] = TOKEN_OPERATOR_XOR,         // ^
    [OPERATOR_NOT] = TOKEN_OPERATOR_NOT,         // !
    [OPERATOR_INCREMENT] = TOKEN_OPERATOR_INCREMENT,   // ++
    [OPERATOR_DECREMENT] = TOKEN_OPERATOR_DECREMENT,   // --
    [OPERATOR_ADD_ASSIGN] = TOKEN_OPERATOR_ADD_ASSIGN,  // +=
    [OPERATOR_SUB_ASSIGN] = TOKEN_OPERATOR_SUB_ASSIGN,  // -=
    [OPERATOR_MUL_ASSIGN] = TOKEN_OPERATOR_MUL_ASSIGN,  // *=
    [OPERATOR_DIV_ASSIGN] = TOKEN_OPERATOR_DIV_ASSIGN,  // /=
    [OPERATOR_MOD_ASSIGN] = TOKEN_OPERATOR_MOD_ASSIGN,  // %=
    [OPERATOR_AND_ASSIGN] = TOKEN_OPERATOR_AND_ASSIGN,  // &=
    [OPERATOR_OR_ASSIGN] = TOKEN_OPERATOR_OR_ASSIGN,   // |=
    [OPERATOR_XOR_ASSIGN] = TOKEN_OPERATOR_XOR_ASSIGN,  // ^=
    [OPERATOR_LEFT_SHIFT] = TOKEN_OPERATOR_LEFT_SHIFT,  // <<
    [OPERATOR_RIGHT_SHIFT] = TOKEN_OPERATOR_RIGHT_SHIFT, // >>
    [OPERATOR_EQUAL] = TOKEN_OPERATOR_EQUAL,       // ==
    [OPERATOR_NOT_EQUAL] = TOKEN_OPERATOR_NOT_EQUAL,   // !=
    [OPERATOR_GREATER_EQUAL] = TOKEN_OPERATOR_GREATER_EQUAL, // >=
    [OPERATOR_LESS_EQUAL] = TOKEN_OPERATOR_LESS_EQUAL,  // <=
    [OPERATOR_ALSO] = TOKEN_OPERATOR_ALSO,        // &&
    [OPERATOR_EITHER] = TOKEN_OPERATOR_EITHER,      // ||
};
OperatorState lookup_operator(unsigned char c) {
    if (c > max_oper_ascii_index) return OPERATOR_ERROR;
    else return operator_lookup[c];
}

void handle_operator(const char* input, int* index, ArrayList* token, State* next_state) {
    OperatorState prev_state = operator_state_table[OPERATOR_START][lookup_operator(input[*index])];
    arraylist_add(token, input[*index]);

    (*index)++;
    OperatorState state = operator_state_table[prev_state][lookup_operator(input[*index])];

    if (state == OPERATOR_ERROR) {
        add_token(token, tokens, STATE_OPERATOR_TO_TOKEN_CONVERTER[prev_state]);
        return;
    }

    arraylist_add(token, input[*index]);
    add_token(token, tokens, STATE_OPERATOR_TO_TOKEN_CONVERTER[state]);
    (*index)++;
}
static const Token_Types SEPARATOR_TO_TOKEN_CONVERTER[] = {
    ['('] = TOKEN_LPAREN,[')'] = TOKEN_RPAREN,
    ['{'] = TOKEN_LBRACES,['}'] = TOKEN_RBRACES,
    ['['] = TOKEN_LBRACES,[']'] = TOKEN_RBRACES,
    [';'] = TOKEN_SEMICOLON,[','] = TOKEN_COMMA,

};
void handle_separator(const char* input, int* index, ArrayList* token, State* next_state) {
    arraylist_add(token, input[*index]);
	add_token(token, tokens, SEPARATOR_TO_TOKEN_CONVERTER[input[*index]]);
    (*index)++;
}

void handle_number(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = NUMBER;
    while (current == NUMBER) {
        arraylist_add(token, input[*index]);
        (*index)++;
        current = state_table[NUMBER][classifier_lookup[input[*index]]];
    }
    if (input[*index] == '.') {
        arraylist_add(token, input[*index]);
        (*index)++;

        current = state_table[NUMBER][classifier_lookup[input[*index]]];

        if (current != NUMBER)
        {
            handle_error(input, index, token, &current);
            return;
        }

        while (current == NUMBER) {
            arraylist_add(token, input[*index]);
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
        arraylist_add(token, input[*index]);
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
        (*index)++;
        current = state_table[START][classifier_lookup[input[*index]]];
    }
}

void handle_identifier(const char* input, int* index, ArrayList* token, State* next_state) {
    State current = state_table[IDENTIFIER][classifier_lookup[input[*index]]];
    while (current == IDENTIFIER) {
        arraylist_add(token, input[*index]);
        (*index)++;
        current = state_table[IDENTIFIER][classifier_lookup[input[*index]]];
    }
	add_token(token, tokens, TOKEN_IDENTIFIER);
}

// FSM for tokenization
Tokens* tokenize(const char* input) {
    State state = START, next_state;
    ArrayList* token = arraylist_init(DEFAULT_TOKEN_SIZE);

    tokens = tokens_init();

    init_keywords();
    int i = 0;
    while (input[i] != '\0') {
        next_state = state_table[state][classifier_lookup[input[i]]];
        call_state_function(input, &i, token, &next_state);
        state = next_state;
    }

    arraylist_free(token);
    stringin_free(keywords_finder);

    return tokens;
}