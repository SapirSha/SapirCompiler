#include "Lexer.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "HashMap.h"
#include "ArrayList.h"
#include "StringIn.h"

#define DEFAULT_TOKEN_SIZE 32

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
} State;

static const State STATE_TO_TOKEN_CONVERTER[NUM_STATES] = {
    [IDENTIFIER] = TOKEN_IDENTIFIER,
    [NUMBER] = TOKEN_NUMBER,
    [OPERATOR] = TOKEN_OPERATOR,
	[SEPARATOR] = TOKEN_SEPARATOR,
    [STRING_LITERAL] = TOKEN_STRING, // New state mapping
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
} CharClass;

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
    "if", "while", "return", "for", "else", "int", "char", NULL
};

StringIn* keywords_finder = NULL;
void init_keywords() {
    if (keywords_finder != NULL) return;

    keywords_finder = stringin_init(); 
    
	for (int i = 0; keywords_list[i] != NULL; i++) {
		stringin_insert_string(keywords_finder, keywords_list[i]);
	}
}

void handle_error(char ch, int line, int col) {
    printf("Error at line %d, col %d: Unrecognized character '%c'\n", line, col, ch);
}
void handle_identifier(const char* input, int* index, ArrayList* token, State* state, State* next_state){
    StringIn* pos = keywords_finder;
    /*
    while (input[*index] != EOF && *next_state == KEYWORD) {
		if (*next_state == ERROR) {
			handle_error(input[*index], 0, 0);
			*state = START;
			arraylist_reset(token);
			return;
		}

        arraylist_add(token, input[*index]);
        if (stringin_next_key(&pos, &input[*index]) == NOT_FOUND) {
            *next_state = IDENTIFIER;
            return;
        }
		(*index)++;

        *state = *next_state;
        CharClass char_class = classifier_lookup[input[*index]];
        *next_state = state_table[KEYWORD][char_class];
    }

    char BACKSLASH_ZERO = '\0'; // Simulating end of string
    if (stringin_next_key(&pos, &BACKSLASH_ZERO) == FOUND)
        *next_state = KEYWORD;
    else
        *next_state = IDENTIFIER;

	(*index)--;
    */
}

// FSM for tokenization
Tokens* tokenize(const char* input) {
    int line = 1;
    int col = 0;


    State state = START;
	ArrayList *token = arraylist_init(DEFAULT_TOKEN_SIZE);

    TokensQueue* tokens = tokens_init();

    init_keywords();

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\n') {
            line++;
			col = -1; // Reset column to -1 because error is only on next stage
        }
        col++;

        char ch = input[i];
        CharClass char_class = classifier_lookup[ch];

        // Get the next state from the table
        State next_state = state_table[state][char_class]; 

        if (next_state == ERROR) {
            handle_error(input[i], line, col);
            state = START;
            arraylist_reset(token);
            continue;
        }

        // Handle state transitions
        if (next_state != state /* || state == OPERATOR */) {
            arraylist_add(token, '\0'); // Null-terminate the token
            char* str_token = token->array;

            if (state == IDENTIFIER || state == KEYWORD || state == NUMBER || state == OPERATOR || state == SEPARATOR) {
                tokens_enqueue(tokens, str_token, STATE_TO_TOKEN_CONVERTER[state]);
            }
            else if (state == STRING_LITERAL) {
                tokens_enqueue(tokens, str_token + 1, TOKEN_STRING);
            }

            arraylist_reset(token);// Reset token for new state
        }



        // Add character to token if in a valid state
        if (next_state == IDENTIFIER || next_state == NUMBER || next_state == OPERATOR || next_state == STRING_LITERAL || next_state == SEPARATOR) {
            arraylist_add(token, ch);
		} else if (next_state == KEYWORD) {
            handle_identifier(input, &i, token, &state, &next_state);
        }

        state = next_state;
    }

    // Emit the last token if any
    if (!array_list_is_empty(token)) {
        arraylist_add(token, '\0'); // Null-terminate the token
        char* str_token = token->array;
        if (state == IDENTIFIER || state == KEYWORD || state == NUMBER || state == OPERATOR || state == SEPARATOR) {
            tokens_enqueue(tokens, str_token, STATE_TO_TOKEN_CONVERTER[state]);
        }
        else if (state == STRING_LITERAL) {
            printf("Error at line %d, col %d: Unclosed string literal. '%c'\n", line, col + 1, '"');
        }
    }

	arraylist_free(token);
	stringin_free(keywords_finder);

    return tokens;
}
