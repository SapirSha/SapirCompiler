#ifndef IR_CFG_H
#define IR_CFG_H

#include "SyntaxTree.h"
#include "ArrayList.h"
#include "Tokens.h"
#include "stdbool.h"
#include "HashSet.h"
#include "LinkedList.h"

typedef enum {
    IR_RAW_STRING,
    IR_GLOBAL_TEMP_SPACE, //
    IR_DECLARE_GLOBAL,//
    IR_DECLARE_LOCAL,//
    IR_ASSIGN,//
    IR_ADD,//
    IR_SUB,//
    IR_MUL,//
    IR_DIV,//
    IR_MOD,//
    IR_LT,//
    IR_LE,//
    IR_GT,//
    IR_GE,//
    IR_EQ,//
    IR_NE,//
    IR_AND,//
    IR_OR,//
    IR_NOT, //
    IR_CBR,//
    IR_FUNC_START,
    IR_FUNC_END,
    IR_RETURN,
    IR_CALL,
    IR_JMP,//
    IR_PRINT,
    IR_PRINT_INT,
    IR_PARAMETER,
    IR_END, //
    IR_GET_INT,
    IT_INST_COUNT,
    OPCODE_LENGTH
} IR_Opcode;

typedef enum {
    IR_NULL = 0,
    IR_TOKEN,
    IR_BLOCK_ID,
    IR_TEMPORARY_ID,
    IR_TOKEN_LIST,
    IR_STR,
    IR_INT,
} IR_DataType;

typedef struct IR_Value{
    IR_DataType type;
    union {
        Token token;
        int num;
        ArrayList* values_list;
        char* str;
    } data;
} IR_Value;

typedef struct IR_Instruction {
    IR_Opcode opcode;
    IR_Value arg1;
    IR_Value arg2;
    IR_Value arg3;
    bool is_live;
}
IR_Instruction;

typedef struct BasicBlock {
    int id;
    ArrayList* instructions;
    ArrayList* successors;
    ArrayList* predecessors;
    HashSet* live_in;
    HashSet* live_out;
} BasicBlock;
BasicBlock* mainBlock;

LinkedList* globalVars;
LinkedList* globalStrings;

unsigned int ir_value_hash(IR_Value* key);
int ir_value_equals(IR_Value* key1, IR_Value* key2);
void printIRValuePointer(IR_Value* val);

void printCFG(BasicBlock* block, int* visited);
BasicBlock* mainCFG(SyntaxTree* tree);


#endif