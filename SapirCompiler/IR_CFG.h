#ifndef IR_CFG_H
#define IR_CFG_H
#include "SyntaxTree.h"
#include "ArrayList.h"
#include "Tokens.h"

typedef enum {
    IR_RAW_STRING,
    IR_DECLARE,
    IR_ASSIGN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_EQ,
    IR_NE,
    IR_AND,
    IR_OR,
    IR_CBR,
    IR_FUNC_START,
    IR_FUNC_END,
    IR_RETURN,
    IR_CALL,
    IR_JMP,
    IT_INST_COUNT
} IR_Opcode;

typedef enum {
    IR_NULL = 0,
    IR_TOKEN,
    IR_BLOCK_ID,
    IR_TEMPORARY_ID,
    IR_TOKEN_LIST,
    IR_STR
} IR_DataType;

typedef struct IR_Value{
    IR_DataType type;
    union {
        Token token;
        int num;
        ArrayList* token_list;
        char* str;
    } data;
} IR_Value;

typedef struct IR_Instruction {
    IR_Opcode opcode;
    IR_Value arg1;
    IR_Value arg2;
    IR_Value arg3;
}
IR_Instruction;

typedef struct BasicBlock {
    int id;
    ArrayList* instructions;
    ArrayList* successors;
    ArrayList* predecessors;
} BasicBlock;
BasicBlock* mainBlock;


BasicBlock* mainCFG(SyntaxTree* tree);


#endif