#include "Lexer.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "HashMap.h"

#define MAX_TOKEN_LENGHT 100

// Define states
typedef enum {
    START,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    ERROR,
    NUM_STATES
} State;

// Define character classes
typedef enum {
    CHAR_INVALID = 0,
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_OPERATOR,
    CHAR_WHITESPACE,
    NUM_CHAR_CLASSES
} CharClass;

// Lookup table for state transitions
static const State state_table[NUM_STATES][NUM_CHAR_CLASSES] = {
    //CHAR_INVALID    CHAR_LETTER      CHAR_DIGIT     CHAR_OPERATOR   CHAR_WHITESPACE  
    { ERROR,          IDENTIFIER,      NUMBER,        OPERATOR,       START,},    // START
    { ERROR,          IDENTIFIER,      IDENTIFIER,    OPERATOR,       START,},    // IDENTIFIER
    { ERROR,          IDENTIFIER,      NUMBER,        OPERATOR,       START,},    // NUMBER
    { ERROR,          IDENTIFIER,      NUMBER,        OPERATOR,       START,},    // OPERATOR
    { ERROR,          ERROR,           ERROR,         ERROR,          ERROR,}     // ERROR
};

// Lookup table for key to class match
static const CharClass classifier_lookup[256] = {
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
	['='] = CHAR_OPERATOR,['>'] = CHAR_OPERATOR,['<'] = CHAR_OPERATOR,['!'] = CHAR_OPERATOR,
	['&'] = CHAR_OPERATOR,['|'] = CHAR_OPERATOR,['^'] = CHAR_OPERATOR,['%'] = CHAR_OPERATOR,
	[';'] = CHAR_OPERATOR,['('] = CHAR_OPERATOR,[')'] = CHAR_OPERATOR,['{'] = CHAR_OPERATOR,
	['}'] = CHAR_OPERATOR,['['] = CHAR_OPERATOR,[']'] = CHAR_OPERATOR,

    //Spaces
	['\n'] = CHAR_WHITESPACE,[' '] = CHAR_WHITESPACE,['\t'] = CHAR_WHITESPACE,
};

static HashMap* keywords_map = NULL;
static const char* keywords_list[] = {
    "if", "while", "return", "for", "else", "int", "char", NULL
};

void init_keywords() {
    if (keywords_map != NULL) return;

	keywords_map = createHashMap(100);

	for (int i = 0; keywords_list[i] != NULL; i++)
		hashmap_insert(keywords_map, keywords_list[i], 1);
}

bool is_keyword(const char* token) {
	return hashmap_exists(keywords_map, token);
}



// FSM for tokenization
void tokenize(const char* input) {
    State state = START;
    char token[MAX_TOKEN_LENGHT];
    int token_index = 0;

    init_keywords();

    for (int i = 0; input[i] != '\0'; i++) {
        char ch = input[i];
        CharClass char_class = classifier_lookup[ch];

        // Get the next state from the table
        State next_state = state_table[state][char_class];

        if (next_state == ERROR) {
            printf("Error: Unrecognized character '%c'\n", ch);
            state = START;
            token_index = 0;
            continue;
        }

        // Handle state transitions
        if (next_state != state) {
            // Emit token if transitioning to START or a different state
            if (state == IDENTIFIER || state == NUMBER || state == OPERATOR) {
                token[token_index] = '\0'; // Null-terminate the token
                if (state == IDENTIFIER) {
                    if (is_keyword(token)) {
                        printf("KEYWORD: %s\n", token);
                    }
                    else {
                        printf("IDENTIFIER: %s\n", token);
                    }
                }
                else if (state == NUMBER) {
                    printf("NUMBER: %s\n", token);
                }
                else if (state == OPERATOR) {
                    printf("OPERATOR: %s\n", token);
                }
            }
            token_index = 0; // Reset token for new state
        }

        // Add character to token if in a valid state
        if (next_state == IDENTIFIER || next_state == NUMBER || next_state == OPERATOR) {
            token[token_index++] = ch;
        }

        state = next_state;
    }

    // Emit the last token if any
    if (token_index > 0) {
        token[token_index] = '\0';
        if (state == IDENTIFIER) {
            printf("IDENTIFIER: %s\n", token);
        }
        else if (state == NUMBER) {
            printf("NUMBER: %s\n", token);
        }
        else if (state == OPERATOR) {
            printf("OPERATOR: %s\n", token);
        }
    }
}


