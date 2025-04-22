#ifndef LEXERFSM_H
#define LEXERFSM_H

#include "Tokens.h"
#include "Boolean.h"

#define VALID_INPUT 1
#define CHAR_POSIBILITIES 128


#define START_STATE STATE0
#define ERROR_STATE ERROR

static enum {
    STATE0,

    NUMBER,

    IDENTIFIER,
    // KEYWORDS:
    B,
    BR,
    BRE,
    BREA,
    BREAK, //BREAK
    BO,
    BOO,
    BOOL, // BOOL
    C,
    CA,
    CAL,
    CALL, // CALL
    CH,
    CHA,
    CHAN,
    CHANG,
    CHANGE, // CHANGE
    D,
    DO, // DO
    E,
    EL,
    ELS,
    ELSE, // ELSE
    F,
    FA,
    FAL,
    FALS,
    FALSE, // FALSE
    FO,
    FOR, // FOR
    FU,
    FUN,
    FUNC,
    FUNCT,
    FUNCTI,
    FUNCTIO,
    FUNCTION, // FUNCTION
    G,
    GE,
    GET, // GET
    GETS, // GETS
    I,
    IF, // IF
    IN,
    INT, // INT
    P,
    PR,
    PRI,
    PRIN,
    PRINT, // PRINT
    R,
    RE,
    RET,
    RETU,
    RETUR,
    RETURN, // RETURN
    RETURNS, // RETURNS
    T,
    TR,
    TRU,
    TRUE, // TRUE
    W,
    WH,
    WHI,
    WHIL,
    WHILE, // WHILE

    // SEPARATORS:
    L_PAREN,
    R_PAREN,
    L_BRACES,
    R_BRACES,
    COMMA,


    // OPERATORS:
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,
    GREATER,
    LESS,
    MODULO,
    NOT,
    AND,
    OR,
    // 2 length:
    EQUAL,
    NOT_EQUAL,
    GREATER_EQUAL,
    LESS_EQUAL,
    ALSO,
    EITHER,

    STRING_LITERAL,
    COMMENT,

    ERROR,

    NUM_OF_STATES
} State;

extern BOOLEAN Available_Lookup[CHAR_POSIBILITIES];
extern enum State Lexer_FSM[NUM_OF_STATES][CHAR_POSIBILITIES];

BOOLEAN is_valid_char(char c);
enum State get_next_state(enum State current_state, char c);
Token_Types convert_state_to_token_type(enum State state);
BOOLEAN is_ignored_state(enum State state);

#endif