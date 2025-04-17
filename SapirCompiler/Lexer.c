#include "Lexer.h"

#include "ErrorHandler.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "ArrayList.h"
#include "StringTrie.h"
#include "Queue.h"

#define DEFAULT_TOKEN_SIZE 32


Queue* tokens = NULL;
int current_line;
int current_line_start_compared_to_index;
int start_line;
int start_line_col_compared_to_index;
bool end_reached;

void new_line(int index) {
    current_line++;
    current_line_start_compared_to_index = index;
}


char* token_to_string(ArrayList* token) {
    static char ZERO = '\0';
    arraylist_add(token, &ZERO);
    char* string = malloc(token->size * sizeof(char));
    if (!string) handle_out_of_memory_error();

    for (int i = 0; i < token->size; i++) {
        string[i] = *(char*)token->array[i];
    }
    return string;
}

void add_token(ArrayList* token, Queue* tokens, Token_Types type, int index) {
    char* str_token = token_to_string(token);
    int line = start_line;
    int col = start_line_col_compared_to_index;

    if (strlen(str_token) > MAX_TOKEN_LENGTH)
        handle_lexical_above_max_token_length(str_token, start_line, start_line_col_compared_to_index);

    queue_enqueue(tokens, &(Token){.lexeme = str_token, .type = type, .row = line, .col = col});
	arraylist_reset(token);
}



void handle_start(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_number(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_keyword(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_operator(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_string_literal(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_comment(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_separator(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_error(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);
void handle_identifier(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state);


void(*states_functions[])(const char*, int*, ArrayList*, LEXER_STATE*) = {
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
void call_state_function(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
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
    CHAR_QUOTE,
    CHAR_COMMENT,
    CHAR_SEPARATOR,
    END_OF_INPUT,
    NUM_CHAR_CLASSES
} CharClass;

// Lookup table for state transitions
static const LEXER_STATE state_table[NUM_STATES][NUM_CHAR_CLASSES] = {
    // State: / Got:        CHAR_INVALID    CHAR_LETTER     CHAR_DIGIT      CHAR_OPERATOR    CHAR_WHITESPACE   CHAR_QUOTE          CHAR_COMMENT     CHAR_SEPARATOR  END_OF_INPUT 
    /* START */           { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR,      START},
    /* IDENTIFIER */      { ERROR,          IDENTIFIER,     IDENTIFIER,     OPERATOR,        START,            START,              COMMENT,          SEPARATOR,      START},
    /* NUMBER */          { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            START,              COMMENT,          SEPARATOR,      START},
    /* OPERATOR */        { ERROR,          KEYWORD,        NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR,      START},
    /* STRING_LITERAL */  { STRING_LITERAL, STRING_LITERAL, STRING_LITERAL, STRING_LITERAL,  STRING_LITERAL,   START,              STRING_LITERAL,   STRING_LITERAL, START},
    /* COMMENT */         { ERROR,          COMMENT,        COMMENT,        COMMENT,         COMMENT,          COMMENT,            START,            COMMENT,        START},
    /* SEPARATOR */       { ERROR,          IDENTIFIER,     NUMBER,         OPERATOR,        START,            STRING_LITERAL,     COMMENT,          STRING_LITERAL, START},
    /* KeyWord */         { ERROR,          KEYWORD,        KEYWORD,        OPERATOR,        START,            STRING_LITERAL,     COMMENT,          SEPARATOR,      START},
    /* ERROR */           { ERROR,          ERROR,          ERROR,          ERROR,           ERROR,            ERROR,              ERROR,            ERROR,          START},
};


// Lookup table for all allowable characters
static const CharClass classifier_lookup[] = {
    // All other characters are invalid
    [0] = END_OF_INPUT,
    [1] = CHAR_INVALID,

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
    [','] = CHAR_SEPARATOR,


    //Spaces
    ['\n'] = CHAR_WHITESPACE,[' '] = CHAR_WHITESPACE,['\t'] = CHAR_WHITESPACE,

    //Comments
    ['#'] = CHAR_COMMENT,

    //Double quote for string literals
    ['"'] = CHAR_QUOTE,

    // All other characters are invalid
    [255] = CHAR_INVALID,
};


void handle_invalid_char(char c, int index) {
    int line = current_line;
    int col = index - current_line_start_compared_to_index;

    handle_unkown_character_error(c, line, col);
}

CharClass get_char_class(char c, int index) {
    if (classifier_lookup[c] == CHAR_INVALID)
        handle_invalid_char(c, index);
    else if (classifier_lookup[c] == END_OF_INPUT)
        end_reached = true;
    return classifier_lookup[c];
}

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
    stringin_insert_string(token_finder, "bool", TOKEN_BOOL);
    stringin_insert_string(token_finder, "true", TOKEN_TRUE);
	stringin_insert_string(token_finder, "false", TOKEN_FALSE);
    stringin_insert_string(token_finder, "do", TOKEN_DO);
    stringin_insert_string(token_finder, "change", TOKEN_CHANGE);
    stringin_insert_string(token_finder, "print", TOKEN_PRINT);
    stringin_insert_string(token_finder, "print_int", TOKEN_PRINT_INT);

    stringin_insert_string(token_finder, "get", TOKEN_GET);
    stringin_insert_string(token_finder, "gets", TOKEN_GETS);
    stringin_insert_string(token_finder, "function", TOKEN_FUNCTION);
    stringin_insert_string(token_finder, "returns", TOKEN_RETURNS);
    stringin_insert_string(token_finder, "return", TOKEN_RETURN);
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
    stringin_insert_string(token_finder, "!", TOKEN_OPERATOR_NOT);

    stringin_insert_string(token_finder, "!=", TOKEN_OPERATOR_NOT_EQUAL);
    stringin_insert_string(token_finder, ">=", TOKEN_OPERATOR_GREATER_EQUAL);
    stringin_insert_string(token_finder, "<=", TOKEN_OPERATOR_LESS_EQUAL);
    stringin_insert_string(token_finder, "&&", TOKEN_OPERATOR_ALSO);
    stringin_insert_string(token_finder, "||", TOKEN_OPERATOR_EITHER);
    stringin_insert_string(token_finder, "==", TOKEN_OPERATOR_EQUAL);
}

void handle_error(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    handle_lexical_error(*next_state, input[*index], current_line, *index - current_line_start_compared_to_index);
    arraylist_reset(token);
    *next_state = START;
    (*index)++;
}

void handle_keyword(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
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
        CharClass next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return; }
        *next_state = state_table[KEYWORD][next_input];
    }

    if (stringin_next(&pos, &clearance, '\0') == FOUND) {
		add_token(token, tokens, pos->is_end, *index);
        *next_state = KEYWORD;
    }
    else {
        *next_state = IDENTIFIER;
        handle_identifier(input, index, token, next_state);
    }
}

void handle_operator(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    StringTrie* pos = token_finder;
    char* clearance = pos->to_clear;

    while (*next_state == OPERATOR) {
        arraylist_add(token, &input[*index]);
        if (stringin_next(&pos, &clearance, input[*index]) == NOT_FOUND) {
            add_token(token, tokens, pos->is_end, *index);
            (*index)++;
            return;
        }
        (*index)++;
        CharClass next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return; }
        *next_state = state_table[OPERATOR][next_input];
    }
    add_token(token, tokens, pos->is_end, *index);
}
// TO BE CHANGED LATER?
static const Token_Types SEPARATOR_TO_TOKEN_CONVERTER[] = {
    ['('] = TOKEN_LPAREN,[')'] = TOKEN_RPAREN,
    ['{'] = TOKEN_LBRACES,['}'] = TOKEN_RBRACES,
    [','] = TOKEN_COMMA,

};
void handle_separator(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    arraylist_add(token, &input[*index]);
	add_token(token, tokens, SEPARATOR_TO_TOKEN_CONVERTER[input[*index]], *index);
    (*index)++;
}

void handle_number(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    LEXER_STATE current = NUMBER;
    while (current == NUMBER) {
        arraylist_add(token, &input[*index]);
        (*index)++;
        CharClass next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return; }
        current = state_table[NUMBER][next_input];
    }
    if (input[*index] == '.') {
        arraylist_add(token, &input[*index]);
        (*index)++;

        CharClass next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return; }
        current = state_table[NUMBER][next_input];

        if (current != NUMBER)
        {
            handle_error(input, index, token, &current);
            return;
        }

        while (current == NUMBER) {
            arraylist_add(token, &input[*index]);
            (*index)++;
            CharClass next_input = get_char_class(input[*index], *index);
            if (next_input == CHAR_INVALID) { (*index)++; return; }
            current = state_table[NUMBER][next_input];
        }
        add_token(token, tokens, TOKEN_FLOAT_NUMBER, *index);
    }
    else {
		add_token(token, tokens, TOKEN_NUMBER, *index);
    }
}

void handle_string_literal(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    LEXER_STATE current;
    (*index)++;
    
    while ((current = state_table[STRING_LITERAL][classifier_lookup[input[*index]]]) == STRING_LITERAL) {
        if (input[*index] != '\n')
            arraylist_add(token, &input[*index]);
        else
            new_line(*index);

        (*index)++;
    }
    if (input[*index] == '"') {
        add_token(token, tokens, TOKEN_STRING_LITERAL, *index);
        (*index)++;
        *next_state = START;
    }
	else {
        out_put_error("--- Error: Unclosed string literal ---\n");
	}
}

void handle_comment(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    LEXER_STATE current = COMMENT;
    while (current == COMMENT) {
        (*index)++;
        CharClass next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return; }
        current = state_table[COMMENT][next_input];
    }
    *next_state = START;
    (*index)++;
}


void handle_start(const char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    LEXER_STATE current = START;
    while (current == START) {
        if (input[*index] == '\n') {
            new_line(*index);
        }
        (*index)++;
        CharClass next_input = get_char_class(input[*index], (*index));
        if (next_input == CHAR_INVALID || next_input == END_OF_INPUT) { (*index)++;  return; }

        current = state_table[START][next_input];

    }
}

void handle_identifier(char* input, int* index, ArrayList* token, LEXER_STATE* next_state) {
    CharClass next_input = get_char_class(input[*index], *index);
    if (next_input == CHAR_INVALID) { (*index)++; return;  }
    LEXER_STATE current = state_table[IDENTIFIER][next_input];
    while (current == IDENTIFIER) {
        arraylist_add(token, &input[*index]);
        (*index)++;
        next_input = get_char_class(input[*index], *index);
        if (next_input == CHAR_INVALID) { (*index)++; return;  }
        current = state_table[IDENTIFIER][get_char_class(input[*index], *index)];
    }

	add_token(token, tokens, TOKEN_IDENTIFIER, *index);
}

Queue* tokenize(const char* input) {
    current_line = 1;
    end_reached = false;
    current_line_start_compared_to_index = 0;
    LEXER_STATE state = START, next_state;
    ArrayList* token = arraylist_init(sizeof(char), DEFAULT_TOKEN_SIZE);

    tokens = queue_init(sizeof(Token));

    init_finder();

    int i = 0;
    while (input[i] != '\0' && !end_reached) {
        start_line = current_line;
        start_line_col_compared_to_index = i - current_line_start_compared_to_index + 1;
        CharClass next_input = get_char_class(input[i], i);
        if (next_input == CHAR_INVALID) { i++; continue; }
        next_state = state_table[START][next_input];
        call_state_function(input, &i, token, &next_state);
        state = next_state;
    }

    queue_enqueue(tokens, &(Token){.type = TOKEN_EOF, .lexeme = "END"});
    queue_enqueue(tokens, &(Token){.type = TOKEN_EOF, .lexeme = "END"});

    arraylist_free(token);
    stringin_free(token_finder);

    return tokens;
}