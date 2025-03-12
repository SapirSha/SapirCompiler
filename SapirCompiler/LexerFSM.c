
/*
#include <stdio.h>

typedef enum {
	UNKNOWN_CHAR = 0,
	LETTER_CHAR,
	DIGIT_CHAR,
	WHITE_SPACE_CHAR,
	OPERATOR_CHAR,
	SEPARATOR_CHAR,
	STRING_LITERAL_CHAR,
	COMMENT_CHAR,
    DOT_CHAR,
    PLUS_CHAR,
    MINUS_CHAR,
    ASTERISK_CHAR,
    SLASH_CHAR,
	NUMBER_OF_TYPES
} _TypeOfInput;

typedef enum {
	START_STATE = 0,
	ERROR_STATE,
    FULL_TOKEN,
	WORD_STATE,
	NUMBER_STATE,
	OPERATOR_STATE,
	SEPARATOR_STATE,
	STRING_STATE,
	COMMENT_STATE,
    FLOAT_STATE,
    PLUS_STATE,
    INCREMENT_STATE,
    MINUS_STATE,
    DECREMENT_STATE,
    MUL_STATE,
    DIV_STATE,
	NUM_OF_STATES
} _State;

typedef enum {
	TOKEN_UNKNOWN = 0,
    

	NUM_OF_TOKENS
} _Token;

static const _State Lexer_FSM[][NUMBER_OF_TYPES] = {
	/*                   UNKNOWN           LETTER           DIGIT             WHITESPACE       OPERATOR           SEPARATOR        STRING_LITERAL    COMMENT         DOT                PLUS              MINUS             ASTERISK           SLASH
	/* START      */ //    {ERROR_STATE,     WORD_STATE,      NUMBER_STATE,     START_STATE,     OPERATOR_STATE,   SEPARATOR_STATE,  STRING_STATE,     COMMENT_STATE,  SEPARATOR_STATE,   PLUS_STATE,       MINUS_STATE,      MUL_STATE,         DIV_STATE},
	/* ERROR      */  //   {ERROR_STATE,     ERROR_STATE,     ERROR_STATE,      ERROR_STATE,     ERROR_STATE,      ERROR_STATE,      ERROR_STATE,      ERROR_STATE,    ERROR_STATE,       ERROR_STATE,      ERROR_STATE,      ERROR_STATE,       ERROR_STATE},
    /* FULL-TOKEN */    // {ERROR_STATE,     WORD_STATE,      NUMBER_STATE,     START_STATE,     OPERATOR_STATE,   SEPARATOR_STATE,  STRING_STATE,     COMMENT_STATE,  SEPARATOR_STATE,   PLUS_STATE,       MINUS_STATE,      MUL_STATE,         DIV_STATE},
    /* WORD       */ //    {FULL_TOKEN,      WORD_STATE,      WORD_STATE,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,        FULL_TOKEN},
	/* NUMBER     */ //    {FULL_TOKEN,      FULL_TOKEN,      NUMBER_STATE,     FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FLOAT_STATE,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,        FULL_TOKEN},
	/* OPERATOR   */ //    {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      OPERATOR_STATE,   FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        OPERATOR_STATE,   OPERATOR_STATE,   OPERATOR_STATE,    OPERATOR_STATE},
	/* SEPERATOR  */ //    {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       SEPARATOR_STATE,  FULL_TOKEN,       FULL_TOKEN,     SEPARATOR_STATE,   FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,        FULL_TOKEN},
	/* STRING     */ //    {STRING_STATE,    STRING_STATE,    STRING_STATE,     STRING_STATE,    STRING_STATE,     STRING_STATE,     FULL_TOKEN,       STRING_STATE,   STRING_STATE,      STRING_STATE,     STRING_STATE,     STRING_STATE,      },
	/* COMMENT    */ //    {COMMENT_STATE,   COMMENT_STATE,   COMMENT_STATE,    COMMENT_STATE,   COMMENT_STATE,    COMMENT_STATE,    COMMENT_STATE,    START_STATE,    COMMENT_STATE,     COMMENT_STATE,    COMMENT_STATE,    COMMENT_STATE},
    /* FLOAT      */ //    {FULL_TOKEN,      FULL_TOKEN,      FLOAT_STATE,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     ERROR_STATE,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN},
    /* +          */    // {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        INCREMENT_STATE,  FULL_TOKEN,       FULL_TOKEN},
    /* ++         */  //   {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN},
    /* -          *///     {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        FULL_TOKEN,       DECREMENT_STATE,  FULL_TOKEN},
    /* --         */  //   {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN},
    /* *          */    // {FULL_TOKEN,      FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,      FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN,     FULL_TOKEN,        FULL_TOKEN,       FULL_TOKEN,       FULL_TOKEN},
    /* /          */     
//};
/*
static const _TypeOfInput Input_Type_Table[] = {
    [0] = UNKNOWN_CHAR,

    //NUMBERS
    ['1'] = DIGIT_CHAR,['2'] = DIGIT_CHAR,['3'] = DIGIT_CHAR,['4'] = DIGIT_CHAR,
    ['5'] = DIGIT_CHAR,['6'] = DIGIT_CHAR,['7'] = DIGIT_CHAR,['8'] = DIGIT_CHAR,
    ['9'] = DIGIT_CHAR,['0'] = DIGIT_CHAR,

    //LETTERS
    ['A'] = LETTER_CHAR,['B'] = LETTER_CHAR,['C'] = LETTER_CHAR,['D'] = LETTER_CHAR,
    ['E'] = LETTER_CHAR,['F'] = LETTER_CHAR,['G'] = LETTER_CHAR,['H'] = LETTER_CHAR,
    ['I'] = LETTER_CHAR,['J'] = LETTER_CHAR,['K'] = LETTER_CHAR,['L'] = LETTER_CHAR,
    ['M'] = LETTER_CHAR,['N'] = LETTER_CHAR,['O'] = LETTER_CHAR,['P'] = LETTER_CHAR,
    ['Q'] = LETTER_CHAR,['R'] = LETTER_CHAR,['S'] = LETTER_CHAR,['T'] = LETTER_CHAR,
    ['U'] = LETTER_CHAR,['V'] = LETTER_CHAR,['W'] = LETTER_CHAR,['X'] = LETTER_CHAR,
    ['Y'] = LETTER_CHAR,['Z'] = LETTER_CHAR,
    ['a'] = LETTER_CHAR,['b'] = LETTER_CHAR,['c'] = LETTER_CHAR,['d'] = LETTER_CHAR,
    ['e'] = LETTER_CHAR,['f'] = LETTER_CHAR,['g'] = LETTER_CHAR,['h'] = LETTER_CHAR,
    ['i'] = LETTER_CHAR,['j'] = LETTER_CHAR,['k'] = LETTER_CHAR,['l'] = LETTER_CHAR,
    ['m'] = LETTER_CHAR,['n'] = LETTER_CHAR,['o'] = LETTER_CHAR,['p'] = LETTER_CHAR,
    ['q'] = LETTER_CHAR,['r'] = LETTER_CHAR,['s'] = LETTER_CHAR,['t'] = LETTER_CHAR,
    ['u'] = LETTER_CHAR,['v'] = LETTER_CHAR,['w'] = LETTER_CHAR,['x'] = LETTER_CHAR,
    ['y'] = LETTER_CHAR,['z'] = LETTER_CHAR,
    ['_'] = LETTER_CHAR,

    //OPERATORS
    ['+'] = PLUS_CHAR,    ['-'] = MINUS_CHAR,['*'] = ASTERISK_CHAR,['/'] = OPERATOR_CHAR,
    ['='] = OPERATOR_CHAR,['>'] = OPERATOR_CHAR,['<'] = OPERATOR_CHAR,['%'] = OPERATOR_CHAR,
    ['&'] = OPERATOR_CHAR,['|'] = OPERATOR_CHAR,['^'] = OPERATOR_CHAR,['!'] = OPERATOR_CHAR,

    //SEPARATORS
    ['('] = SEPARATOR_CHAR,[')'] = SEPARATOR_CHAR,['{'] = SEPARATOR_CHAR,['}'] = SEPARATOR_CHAR,
    ['['] = SEPARATOR_CHAR,[']'] = SEPARATOR_CHAR,[';'] = SEPARATOR_CHAR,[','] = SEPARATOR_CHAR,

    ['.'] = DOT_CHAR,


    //Spaces
    ['\n'] = WHITE_SPACE_CHAR,[' '] = WHITE_SPACE_CHAR,['\t'] = WHITE_SPACE_CHAR,

    //Comments
    ['#'] = COMMENT_CHAR,

    //Double quote for string literals
    ['"'] = STRING_LITERAL_CHAR,


    // All other characters are invalid
    [255] = UNKNOWN_CHAR,
};

#define CONVERT_INPUT_TO_TYPE(input) Input_Type_Table[input]

int main() {
	printf("LEXER\n");

    _State s = START_STATE;

    printf("Current State: %d\n", s);
    s = Lexer_FSM[s][CONVERT_INPUT_TO_TYPE('5')];

    printf("Current State: %d\n", s);
    s = Lexer_FSM[s][CONVERT_INPUT_TO_TYPE('.')];


    printf("Current State: %d\n", s);
    s = Lexer_FSM[s][CONVERT_INPUT_TO_TYPE('7')];

    printf("Current State: %d\n", s);
    s = Lexer_FSM[s][CONVERT_INPUT_TO_TYPE(' ')];

    printf("Current State: %d\n", s);



	return 0;
}

*/