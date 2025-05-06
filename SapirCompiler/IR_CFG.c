#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     
#include "SyntaxTree.h"
#include "Tokens.h"
#include "SymbolTable.h"
#include "Sementic.h"
#include "IR_CFG.h"
#include "LinkedList.h"
#include "HashMap.h"
#include "ArrayList.h"
#include <stdbool.h>
#include "ErrorHandler.h"
#include "Parser.h"

#define NONTERMINAL_COUNT_DEFUALT2 100

#pragma warning(disable:4996)

static HashMap* ir_visitor = NULL;

static HashMap* opcode_size;
static HashMap* token_size;

static void init_variable_sizes() {
    static int size1 = 1;
    static int size2 = 2;

    opcode_size = createHashMap(10, int_hash, int_equals);
    token_size = createHashMap(10, int_hash, int_equals);

    hashmap_insert(opcode_size, new_int_with_allocation(IR_LE), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_LT), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_GT), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_GE), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_OR), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_AND), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_EQ), &size1);
    hashmap_insert(opcode_size, new_int_with_allocation(IR_NE), &size1);

    hashmap_insert(token_size, new_int_with_allocation(TOKEN_NUMBER), &size2);
    hashmap_insert(token_size, new_int_with_allocation(TOKEN_TRUE), &size1);
    hashmap_insert(token_size, new_int_with_allocation(TOKEN_FALSE), &size1);
}

static int size_of_token(Token* token) {
    if (hashmap_get(token_size, &token->type) != NULL)
        return *(int*)hashmap_get(token_size, &token->type);
    if (token->type == TOKEN_IDENTIFIER) {
        SymbolInfo* sym_info = (SymbolInfo*)hashmap_get(symbol_table->SymbolMap, token->lexeme);
        return sym_info->size;
    }
    return 2;
}

static int size_of_ir_value(IR_Value v1) {
    if (v1.type == IR_TEMPORARY_ID) {
        TempSymbolInfo* tempinfo = hashmap_get(symbol_table->TemporaryVarMap, &v1.data.num);
        return tempinfo->size;
    }
    if (v1.type == IR_TOKEN) {
        return size_of_token(&v1.data.token);
    }
    return 2;
}

static int temporary_size(IR_Value v1, IR_Value v2, IR_Opcode opcode) {
    if (hashmap_get(opcode_size, &opcode) != NULL)
        return *(int*)hashmap_get(opcode_size, &opcode);
    
    int size1 = size_of_ir_value(v1);
    int size2 = size_of_ir_value(v2);

    return size1 > size2 ? size1 : size2;
}


unsigned int ir_value_hash(IR_Value* key) {
	if (key->type == IR_TOKEN) {
		return string_hash(key->data.token.lexeme);
	}
	else if (key->type == IR_BLOCK_ID || key->type == IR_TEMPORARY_ID) {
		return int_hash(&key->data.num);
	}
	else if (key->type == IR_TOKEN_LIST) {
		return key->data.values_list->object_size * 31 + key->data.values_list->size * 117;
	}
	else if (key->type == IR_STR) {
		return string_hash(key->data.str);
	}
    return (int)key * 31;
}

int ir_value_equals(IR_Value* key1, IR_Value* key2) {
    if (key1->type != key2->type) {
        return 0;
    }
    switch (key1->type) {
    case IR_TOKEN:
        return strcmp(key1->data.token.lexeme, key2->data.token.lexeme) == 0;
    case IR_BLOCK_ID:
    case IR_TEMPORARY_ID:
        return key1->data.num == key2->data.num;
    case IR_TOKEN_LIST:
        return arraylist_equals(key1->data.values_list, key2->data.values_list);
    case IR_STR:
        return strcmp(key1->data.str, key2->data.str) == 0;
    default:
        return 0;
    }
}

LinkedList* function_entry_blocks = NULL;
LinkedList* function_exit_blocks = NULL; 

IR_Instruction* createIRInstructionBase() {
	IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    if (!instr) {
        handle_out_of_memory_error();
        return NULL;
    }

	instr->arg1.type = IR_NULL;
	instr->arg2.type = IR_NULL;
	instr->arg3.type = IR_NULL;
	instr->opcode = IR_NULL;
	instr->arg1.data.num = 0;
	instr->arg2.data.num = 0;
	instr->arg3.data.num = 0;

    instr->is_live = false;
    return instr;
}

IR_Instruction* createGlobalTemps() {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_GLOBAL_TEMP_SPACE;

    instr->arg1.type = IR_INT;
    instr->arg1.data.num = 0;

    return instr;
}

IR_Instruction* setGlobalTemps(IR_Instruction* instr, int size) {
    instr->arg1.type = IR_INT;
    instr->arg1.data.num = size;

    return instr;
}


IR_Instruction* createRawIRInstruction(const char* text) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = IR_RAW_STRING;

    instr->arg1.type = IR_STR;
    instr->arg1.data.str = strdup(text);
    
    handle_other_errors("\n\t---INVALID INSTRUCTION DETECTED\n");
    return instr;
}


IR_Instruction* createAssignInstruction(Token dest, IR_Value src) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = IR_ASSIGN;

    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = dest;

    instr->arg2 = src;

    return instr;
}

IR_Instruction* createEndInstruction() {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_END;

    return instr;
}

void setFunctionCurrnetOffsetInstruction(IR_Instruction* instr, int number_of_params) {
    instr->arg2.type = IR_INT;
    instr->arg2.data.num = number_of_params;
}

void setFunctionParamOffsetInstruction(IR_Instruction* instr, int number_of_params) {
    instr->arg3.type = IR_INT;
    instr->arg3.data.num = number_of_params;
}


IR_Instruction* createBinaryOpInstruction(IR_Opcode op, IR_Value temp, IR_Value left, IR_Value right) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = op;
    
    instr->arg1 = temp;


    if (function_entry_blocks->size != 0) {
        IR_Instruction* func_enter =
            *(IR_Instruction**)
            ((*(CodeBlock**)linkedlist_peek(function_entry_blocks))->instructions->array[0]);
        int* pos = malloc(sizeof(int));
        if (!pos) {
            handle_out_of_memory_error();
            return NULL;
        }

        *pos = func_enter->arg2.data.num;


        int size = temporary_size(left, right, op);
        int align = align_size(size);

        int aligned = (*pos + align - 1) / align * align;
        int offset = -(aligned + size);



        TempSymbolInfo* info = malloc(sizeof(TempSymbolInfo));
        if (!info) {
            handle_out_of_memory_error();
            return NULL;
        }

        info->id = temp.data.num;
        info->size = size;
        info->alignment = align;
        info->offset = offset;
        info->local = 1;


        hashmap_insert(symbol_table->TemporaryVarMap, &info->id, info);

        *pos = aligned + size;
        setFunctionCurrnetOffsetInstruction(func_enter, *pos);
    }
    else {

        int id = temp.data.num;
        int temp_size = temporary_size(left, right, op);
        int temp_align = align_size(temp_size);


        int aligned = (symbol_table->temporary_vars_offset + temp_align - 1) / temp_align * temp_align;
        int offset = -(aligned + temp_size);
        symbol_table->temporary_vars_offset = aligned + temp_size;

        setGlobalTemps(*(IR_Instruction**)mainBlock->instructions->array[0], aligned + temp_size);

        TempSymbolInfo* info = malloc(sizeof(TempSymbolInfo));
        if (!info) {
            handle_out_of_memory_error();
            return NULL;
        }

        info->id = id;
        info->size = temp_size;
        info->alignment = temp_align;
        info->offset = offset;
        info->local = 0;

        hashmap_insert(symbol_table->TemporaryVarMap, &info->id, info);

    }

    instr->arg2 = left;

    instr->arg3 = right;

    return instr;
}




IR_Instruction* createDeclareInstruction(Token name) {
    IR_Instruction* instr  = createIRInstructionBase();
    SymbolInfo* symbol_info = (SymbolInfo*)hashmap_get(symbol_table->SymbolMap, name.lexeme);

    if (function_entry_blocks->size != 0) {
        instr->opcode = IR_DECLARE_LOCAL;

        IR_Instruction* func_enter =
            *(IR_Instruction**)
            ((*(CodeBlock**)linkedlist_peek(function_entry_blocks))->instructions->array[0]);
        int* pos = malloc(sizeof(int));
        if (!pos) {
            handle_out_of_memory_error();
            return NULL;
        }

        *pos = func_enter->arg2.data.num;

        symbol_info->local = 1;
        
        int size = symbol_info->size;
        int align = symbol_info->alignment;
        
        int aligned = (*pos + align - 1) / align * align;   
        int offset = -(aligned + size);   
        symbol_info->offset = offset;

        *pos = aligned + size;   
        setFunctionCurrnetOffsetInstruction(func_enter, *pos);
    }
    else {
        symbol_info->local = 0;
        instr->opcode = IR_DECLARE_GLOBAL;

        linkedlist_push(globalVars, &name);
    }

    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = name;



    return instr;
}



IR_Instruction* createConditionalBranchInstruction(IR_Value temp_id, int then_id, int after_or_else_id) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = IR_CBR;
    instr->arg1 = temp_id;

    instr->arg2.type = IR_BLOCK_ID;
    instr->arg2.data.num = then_id;

    instr->arg3.type = IR_BLOCK_ID;
    instr->arg3.data.num = after_or_else_id;

    return instr;
}

IR_Instruction* createFunctionLimitsInstruction(IR_Opcode op, Token name) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = op;
    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = name;

    instr->arg2.type = IR_INT; // space
    instr->arg2.data.num = 0;

    instr->arg2.type = IR_INT; // space for params
    instr->arg2.data.num = 0;
    
    
    return instr;
}



IR_Instruction* createReturnInstruction(IR_Value ret_val, int end_id) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = IR_RETURN;
    
    instr->arg1 = ret_val;

    instr->arg2.type = IR_BLOCK_ID;
    instr->arg2.data.num = end_id;

    return instr;
}

IR_Instruction* createCallInstruction(int func_start_id, ArrayList* arg_list, IR_Value dest_temp) {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_CALL;
    instr->arg1.type = IR_BLOCK_ID;
    instr->arg1.data.num = func_start_id;
    instr->arg2.type = IR_TOKEN_LIST;
    instr->arg2.data.values_list = arg_list;
    instr->arg3 = dest_temp;


    if (function_entry_blocks->size != 0) {
        IR_Instruction* func_enter =
            *(IR_Instruction**)
            ((*(CodeBlock**)linkedlist_peek(function_entry_blocks))->instructions->array[0]);
        int* pos = malloc(sizeof(int));
        if (!pos) {
            handle_out_of_memory_error();
            return NULL;
        }
        *pos = func_enter->arg2.data.num;
        int size = 2;
        int align = align_size(size);
        int aligned = (*pos + align - 1) / align * align;
        int offset = -(aligned + size);
        TempSymbolInfo* info = malloc(sizeof(TempSymbolInfo));
        if (!info) {
            handle_out_of_memory_error();
            return NULL;
        }
        info->id = dest_temp.data.num;
        info->size = size;
        info->alignment = align;
        info->offset = offset;
        info->local = 1;
        hashmap_insert(symbol_table->TemporaryVarMap, &info->id, info);
        *pos = aligned + size;
        setFunctionCurrnetOffsetInstruction(func_enter, *pos);
    }
    else {

        int id = dest_temp.data.num;
        int temp_size = 2;
        int temp_align = align_size(temp_size);


        int aligned = (symbol_table->temporary_vars_offset + temp_align - 1) / temp_align * temp_align;
        int offset = -(aligned + temp_size);
        symbol_table->temporary_vars_offset = aligned + temp_size;

        setGlobalTemps(*(IR_Instruction**)mainBlock->instructions->array[0], aligned + temp_size);

        TempSymbolInfo* info = malloc(sizeof(TempSymbolInfo));
        if (!info) {
            handle_out_of_memory_error();
            return NULL;
        }

        info->id = id;
        info->size = temp_size;
        info->alignment = temp_align;
        info->offset = offset;
        info->local = 0;

        hashmap_insert(symbol_table->TemporaryVarMap, &info->id, info);
    }



    return instr;
}

IR_Instruction* createJumpInstruction(int block_id) {
    IR_Instruction* instr  = createIRInstructionBase();
    instr->opcode = IR_JMP;
    instr->arg1.type = IR_BLOCK_ID;
    instr->arg1.data.num = block_id;
    return instr;
}

IR_Instruction* createPrintInstruction(int string_id) {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_PRINT;

    instr->arg1.type = IR_INT;
    instr->arg1.data.num = string_id;

    return instr;
}

IR_Instruction* createPrintIntInstruction(IR_Value exp) {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_PRINT_INT;
    instr->arg1 = exp;

    return instr;
}

IR_Instruction* createGetIntInstruction(Token exp) {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_GET_INT;
    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = exp;

    return instr;
}

IR_Instruction* createParameterInstruction(Token param, int param_num) {
    IR_Instruction* instr = createIRInstructionBase();
    instr->opcode = IR_PARAMETER;

    IR_Instruction* func_enter =
        *(IR_Instruction**)
        ((*(CodeBlock**)linkedlist_peek(function_entry_blocks))->instructions->array[0]);
    int* pos = malloc(sizeof(int));
    if (!pos) {
        handle_out_of_memory_error();
        return NULL;
    }

    *pos = func_enter->arg2.data.num;

    SymbolInfo* symbol_info = (SymbolInfo*)hashmap_get(symbol_table->SymbolMap, param.lexeme);
    symbol_info->local = 1;

    int size = symbol_info->size;
    int align = symbol_info->alignment;

    int aligned = (*pos + align - 1) / align * align;
    int offset = -(aligned + size);
    symbol_info->offset = offset;
    *pos = aligned + size;

    setFunctionCurrnetOffsetInstruction(func_enter, *pos);
    setFunctionParamOffsetInstruction(func_enter, *pos);

    instr->arg1.type = IR_INT;
    instr->arg1.data.num = offset;


    instr->arg2.type = IR_TOKEN;
    instr->arg2.data.token = param;

    return instr;
}

static int tempCounter = 0;
static int newTempCounter() {
    return tempCounter++;
}

static IR_Value newTemp() {
    IR_Value* temporary = malloc(sizeof(IR_Value));
    if (!temporary) {
        handle_out_of_memory_error();
        return (IR_Value) { .type = IR_RAW_STRING, .data.str = "MEMORY ERROR" };
    }

    temporary->type = IR_TEMPORARY_ID;
    temporary->data.num = tempCounter++;
    return *temporary;
}



CodeBlock* buildCFG(SyntaxTree* tree, CodeBlock* current);

int next_block_id = 0;
CodeBlock* createBasicBlock(void) {
    CodeBlock* block = (CodeBlock*)malloc(sizeof(CodeBlock));
    if (!block) {
        handle_out_of_memory_error();
        return NULL;
    }

    block->id = next_block_id++;
    block->instructions = arraylist_init(sizeof(IR_Instruction*), 8);
    block->successors = arraylist_init(sizeof(CodeBlock*), 4);
    block->predecessors = arraylist_init(sizeof(CodeBlock*), 4);
	block->live_in = hashset_create(16, ir_value_hash, ir_value_equals);
    block->live_out = hashset_create(16, ir_value_hash, ir_value_equals);

    return block;
}

void addIRInstruction(CodeBlock* block, IR_Instruction* instr) {
    arraylist_add(block->instructions, &instr);
}

void addPredecessor(CodeBlock* block, CodeBlock* pred) {
    arraylist_add(block->predecessors, &pred);
}

void addSuccessor(CodeBlock* block, CodeBlock* succ) {
    arraylist_add(block->successors, &succ);
    addPredecessor(succ, block);
}

typedef struct {
    char* name;
    CodeBlock* entry;
    CodeBlock* exit;
    int num_of_params;
} FunctionCFGEntry;

FunctionCFGEntry* functionCFGTable = NULL;
int functionCFGCount = 0;

void addFunctionCFG(const char* name, CodeBlock* entry, CodeBlock* exit) {
    FunctionCFGEntry* temp = functionCFGTable;
    functionCFGTable = realloc(functionCFGTable, sizeof(FunctionCFGEntry) * (functionCFGCount + 1));
    if (!functionCFGTable) {
        free(temp);
        handle_out_of_memory_error();
        return;
    }
    functionCFGTable[functionCFGCount].name = strdup(name);
    functionCFGTable[functionCFGCount].entry = entry;
    functionCFGTable[functionCFGCount].exit = exit;
    functionCFGTable[functionCFGCount].num_of_params = 0;
    functionCFGCount++;
}

int find_function_index(const char* name) {
    for (int i = 0; i < functionCFGCount; i++) {
        if (strcmp(name, functionCFGTable[i].name) == 0)
            return i;
    }
    return -1;
}


static HashMap* operatorMap = NULL;

#define INSERT_OPCODE(op, str)                                         \
    {                                                                   \
        IR_Opcode* tmp = (IR_Opcode*)malloc(sizeof(IR_Opcode));          \
        if (!tmp) { handle_out_of_memory_error(); return IR_RAW_STRING; } \
        *tmp = (op);                                                       \
        hashmap_insert(operatorMap, (str), tmp);                            \
    }

static IR_Opcode mapComparisonOperator(char* opStr) {
    if (operatorMap == NULL) {
        operatorMap = createHashMap(32, string_hash, string_equals);
        INSERT_OPCODE(IR_LT, "<");
        INSERT_OPCODE(IR_LE, "<=");
        INSERT_OPCODE(IR_GT, ">");
        INSERT_OPCODE(IR_GE, ">=");
        INSERT_OPCODE(IR_EQ, "==");
        INSERT_OPCODE(IR_NE, "!=");
        INSERT_OPCODE(IR_OR, "||");
        INSERT_OPCODE(IR_AND, "&&");
        INSERT_OPCODE(IR_ADD, "+");
        INSERT_OPCODE(IR_SUB, "-");
        INSERT_OPCODE(IR_MUL, "*");
        INSERT_OPCODE(IR_DIV, "/");
        INSERT_OPCODE(IR_MOD, "%");
    }
    
    IR_Opcode* res = (IR_Opcode*)hashmap_get(operatorMap, opStr);

    return res ? *res : IR_RAW_STRING;
}

IR_Value lowerExpression(SyntaxTree* exprTree, CodeBlock** current);

void find_arguments_for_call(SyntaxTree* tree, CodeBlock** current, ArrayList* list) {
    if (strcmp(tree->info.nonterminal_info.nonterminal, "ARGUMENT_LIST") == 0) {
        find_arguments_for_call(tree->info.nonterminal_info.children[0], current, list);

        IR_Value arg = lowerExpression(tree->info.nonterminal_info.children[2], current);

        arraylist_add(list, &arg);
        return;
    }

    IR_Value arg = lowerExpression(tree, current);

    arraylist_add(list, &arg);
}

IR_Value lowerFunctionCall(SyntaxTree* exprTree, CodeBlock** current) {
    char* funcName = exprTree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
    int found = find_function_index(funcName);

    ArrayList* call_arguments = arraylist_init(sizeof(IR_Value), 3);

    if (exprTree->info.nonterminal_info.num_of_children > 2) {
        find_arguments_for_call(exprTree->info.nonterminal_info.children[3], current, call_arguments);
    }


    IR_Value temp = newTemp();
    functionCFGTable[found].entry->id;
	IR_Instruction* instr = 
        createCallInstruction(functionCFGTable[found].entry->id,
            call_arguments, temp);

    addIRInstruction(*current, instr);

    addSuccessor(*current, functionCFGTable[found].entry);
    CodeBlock* cont = createBasicBlock();
    addSuccessor(functionCFGTable[found].exit, cont);

    addIRInstruction(*current, createJumpInstruction(cont->id));

    *current = cont;

    return temp;
}

IR_Value lowerExpression(SyntaxTree* exprTree, CodeBlock** current) {
    if (exprTree->type == TERMINAL_TYPE) {
        IR_Value return_value;
        return_value.type = IR_TOKEN;
        return_value.data.token = exprTree->info.terminal_info.token;
    
        return return_value;
    }
   
    if (strcmp(exprTree->info.nonterminal_info.nonterminal, "FUNCTION_CALL_STATEMENT") == 0 ||
        strcmp(exprTree->info.nonterminal_info.nonterminal, "FUNCTION_CALL_WITH_NOTHING_STATEMENT") == 0) {
        return lowerFunctionCall(exprTree, current);
    }

    if (exprTree->info.nonterminal_info.num_of_children == 3) {
        if (strcmp(exprTree->info.nonterminal_info.nonterminal, "FACTOR") == 0) {
            return lowerExpression(exprTree->info.nonterminal_info.children[1], current);
        }
        SyntaxTree* leftChild = exprTree->info.nonterminal_info.children[0];
        SyntaxTree* opChild = exprTree->info.nonterminal_info.children[1];
        SyntaxTree* rightChild = exprTree->info.nonterminal_info.children[2];

        IR_Value leftValue = lowerExpression(leftChild, current);
        IR_Value rightValue = lowerExpression(rightChild, current);

        char* opStr = opChild->info.terminal_info.token.lexeme;
        IR_Opcode opCode = mapComparisonOperator(opStr);

        IR_Value temp = newTemp();

        IR_Instruction* binInstr = createBinaryOpInstruction(opCode, temp, leftValue, rightValue);
        addIRInstruction(*current, binInstr);

        return temp;
    }

    handle_other_errors("\t\n---INVALID EXPRESSION STATEMENT");
    exit(-1);
}


CodeBlock* return_block(SyntaxTree* tree, CodeBlock* current) {
    IR_Value retVal;
    if (tree->info.nonterminal_info.num_of_children > 1) {
        retVal = lowerExpression(tree->info.nonterminal_info.children[1], &current);
    }
    else {
        retVal.type = IR_NULL;
    }

    CodeBlock* exitBlock = *(CodeBlock**)(linkedlist_peek(function_exit_blocks));
    IR_Instruction* retInstr = createReturnInstruction(retVal, exitBlock->id);
    addIRInstruction(current, retInstr);

    addSuccessor(current, exitBlock);
    return current;
}

CodeBlock* assignment_block(SyntaxTree* tree, CodeBlock* current) {
    Token dest = tree->info.nonterminal_info.children[0]->info.terminal_info.token;
    IR_Value exprResult = lowerExpression(tree->info.nonterminal_info.children[2], &current);

    IR_Instruction* assignInstr = createAssignInstruction(dest, exprResult);
    addIRInstruction(current, assignInstr);

    return current;
}

CodeBlock* decl_block(SyntaxTree* tree, CodeBlock* current) {
    Token name = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createDeclareInstruction(name));

    return current;
}

CodeBlock* decl_assignment_block(SyntaxTree* tree, CodeBlock* current) {
    Token name = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createDeclareInstruction(name));

    IR_Value exprResult = lowerExpression(tree->info.nonterminal_info.children[3], &current);
    IR_Instruction* assignInstr = createAssignInstruction(name, exprResult);
    addIRInstruction(current, assignInstr);

    if (functionCFGCount != 0) {
        functionCFGTable[functionCFGCount - 1].num_of_params++;
    }
    else {

    }

    return current;
}

void lower_parameter(SyntaxTree* tree, CodeBlock* block, int index) {
	SyntaxTree* param = tree->info.nonterminal_info.children[1];
	IR_Instruction* ir = createParameterInstruction(param->info.terminal_info.token, index);
	addIRInstruction(block, ir);
}

void lower_parameter_list(SyntaxTree* tree, CodeBlock* block, int* param_index) {
    if (strcmp(tree->info.nonterminal_info.nonterminal, "PARAMETER_LIST") == 0) {
        lower_parameter_list(tree->info.nonterminal_info.children[0], block, param_index);
        (*param_index)++;

		lower_parameter(tree->info.nonterminal_info.children[2], block, *param_index);
    }
    else {
        lower_parameter(tree, block, *param_index);
    }
}


FunctionCFGEntry* buildFunctionCFG(SyntaxTree* tree) {
    Token funcName = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    CodeBlock* entry = createBasicBlock();
    linkedlist_push(function_entry_blocks, &entry);

    IR_Instruction* entry_instr = createFunctionLimitsInstruction(IR_FUNC_START, funcName);
    addIRInstruction(entry, entry_instr);


    if (strcmp(tree->info.nonterminal_info.nonterminal, "FUNCTION_DECLARATION_STATEMENT") == 0
        || strcmp(tree->info.nonterminal_info.nonterminal, "FUNCTION_DECLARATION_NO_RETURN_STATEMENT") == 0) {
        int num_of_params = 0;
        lower_parameter_list(tree->info.nonterminal_info.children[3], entry, &num_of_params);
    }
    
    CodeBlock* exit = createBasicBlock();
    linkedlist_push(function_exit_blocks, &exit);
    int function_index = functionCFGCount;
    addFunctionCFG(funcName.lexeme, entry, exit);

    

    CodeBlock* bodyEnd = buildCFG(tree->info.nonterminal_info.children[tree->info.nonterminal_info.num_of_children - 1], entry);
    addSuccessor(bodyEnd, exit);
    addIRInstruction(bodyEnd, createJumpInstruction(exit->id));


    IR_Instruction* func_enter =
        *(IR_Instruction**)
        ((*(CodeBlock**)linkedlist_peek(function_entry_blocks))->instructions->array[0]);
    int space = func_enter->arg2.data.num;
    int params_space = func_enter->arg3.data.num;

    IR_Instruction* end_instr = createFunctionLimitsInstruction(IR_FUNC_END, funcName);
    setFunctionCurrnetOffsetInstruction(end_instr, space);
    setFunctionParamOffsetInstruction(end_instr, params_space);

    addIRInstruction(exit, end_instr);


    linkedlist_pop(function_exit_blocks);
    linkedlist_pop(function_entry_blocks);




    FunctionCFGEntry* fc = (FunctionCFGEntry*)malloc(sizeof(FunctionCFGEntry));
    if (!fc) {
        handle_out_of_memory_error();
        return NULL;
    }

    fc->name = strdup(funcName.lexeme);
    if (!fc->name) handle_out_of_memory_error();

    fc->entry = entry;
    fc->exit = exit;
    return fc;
}

CodeBlock* if_block(SyntaxTree* tree, CodeBlock* current) {
    CodeBlock* thenBlock = createBasicBlock();
    CodeBlock* then_start = thenBlock;
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);

    CodeBlock* mergeBlock = createBasicBlock();

    addSuccessor(current, then_start);
    addSuccessor(current, mergeBlock);
    addSuccessor(thenBlock, mergeBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &current);

    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        then_start->id,
        mergeBlock->id);

    addIRInstruction(current, cbrInstr);

    addIRInstruction(thenBlock, createJumpInstruction(mergeBlock->id));

    return mergeBlock;
}

CodeBlock* if_else_block(SyntaxTree* tree, CodeBlock* current) {

    CodeBlock* thenBlock = createBasicBlock();
    CodeBlock* then_start = thenBlock;

    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);
    CodeBlock* elseBlock = createBasicBlock();
    CodeBlock* else_start = elseBlock;

    elseBlock = buildCFG(tree->info.nonterminal_info.children[4], elseBlock);
    CodeBlock* mergeBlock = createBasicBlock();

    addSuccessor(current, then_start);
    addSuccessor(current, else_start);
    addSuccessor(thenBlock, mergeBlock);
    addSuccessor(elseBlock, mergeBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &current);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        then_start->id,
        else_start->id);
    addIRInstruction(current, cbrInstr);

    addIRInstruction(thenBlock, createJumpInstruction(mergeBlock->id));
    addIRInstruction(elseBlock, createJumpInstruction(mergeBlock->id));

    return mergeBlock;
}

CodeBlock* while_block(SyntaxTree* tree, CodeBlock* current) {
    CodeBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));


    CodeBlock* loopBody = createBasicBlock();
    CodeBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[2], loopBody);

    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, loopHeader);
    CodeBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

CodeBlock* do_while_block(SyntaxTree* tree, CodeBlock* current) {
    CodeBlock* loopBody = createBasicBlock();
    CodeBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[1], loopBody);
    addSuccessor(current, body_start);

    addIRInstruction(current, createJumpInstruction(body_start->id));

    CodeBlock* loopHeader = createBasicBlock();
    addSuccessor(loopBody, loopHeader);
    addSuccessor(loopHeader, body_start);
    CodeBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

CodeBlock* block_block(SyntaxTree* tree, CodeBlock* current) {
    if (tree->info.nonterminal_info.num_of_children > 1)
        return buildCFG(tree->info.nonterminal_info.children[1], current);

    return current;
}

CodeBlock* for_block(SyntaxTree* tree, CodeBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    CodeBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));

    CodeBlock* loopBody = createBasicBlock();
    CodeBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, loopHeader);
    CodeBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

CodeBlock* for_change_block(SyntaxTree* tree, CodeBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    CodeBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));

    CodeBlock* loopBody = createBasicBlock();
    CodeBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    CodeBlock* changeBlock = createBasicBlock();
    changeBlock = buildCFG(tree->info.nonterminal_info.children[6], changeBlock);
    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, changeBlock);
    addSuccessor(changeBlock, loopHeader);
    CodeBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(changeBlock->id));
    addIRInstruction(changeBlock, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

CodeBlock* function_block(SyntaxTree* tree, CodeBlock* current) {
    FunctionCFGEntry* fc = buildFunctionCFG(tree);
    free(fc->name);
    free(fc);
    return current;
}

CodeBlock* call_block(SyntaxTree* tree, CodeBlock* current) {
    lowerFunctionCall(tree, &current);
    return current;
}

CodeBlock* statements_block(SyntaxTree* tree, CodeBlock* current) {
    for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
        current = buildCFG(tree->info.nonterminal_info.children[i], current);
    }
    return current;
}

CodeBlock* defualt_block(SyntaxTree* tree, CodeBlock* current) {
    for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
        current = buildCFG(tree->info.nonterminal_info.children[i], current);
    }
    return current;
}

CodeBlock* (*get_block_fun(char* str))(SyntaxTree*, CodeBlock*) {
    CodeBlock* (*pointer)(SyntaxTree*, CodeBlock*) = hashmap_get(ir_visitor, str);
    if (pointer == NULL) {
        return &defualt_block;
    }
    else {
        return pointer;
    }
}

CodeBlock* buildCFG(SyntaxTree* tree, CodeBlock* current) {
    if (!tree)
        return current;

    char* nonterminal = tree->info.nonterminal_info.nonterminal;

    return (*get_block_fun(nonterminal))(tree, current);
}

CodeBlock* print_block(SyntaxTree* tree, CodeBlock* current) { // string
    Token src = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    linkedlist_add(globalStrings, &src);
    
    StringInfo* info = hashmap_get(symbol_table->GlobalStrings, src.lexeme);

    IR_Instruction* printInstr = createPrintInstruction(info->id);
	addIRInstruction(current, printInstr);

	return current; 
}

CodeBlock* print_int_block(SyntaxTree* tree, CodeBlock* current) {
    IR_Value exprResult = lowerExpression(tree->info.nonterminal_info.children[1], &current);
    IR_Instruction* printInstr = createPrintIntInstruction(exprResult);
    addIRInstruction(current, printInstr);
    return current; 
}

CodeBlock* get_decl_block(SyntaxTree* tree, CodeBlock* current) {
    SyntaxTree* var_decl = tree->info.nonterminal_info.children[1];
    decl_block(var_decl, current);
    Token name = var_decl->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createGetIntInstruction(name));

    return current;
}

CodeBlock* get_block(SyntaxTree* tree, CodeBlock* current) {
    Token name = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createGetIntInstruction(name));
	return current;
}


static init_ir_visitor() {
	hashmap_insert(ir_visitor, "STATEMENTS", &statements_block);
	hashmap_insert(ir_visitor, "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", &decl_assignment_block);
	hashmap_insert(ir_visitor, "VARIABLE_ASSIGNMENT_STATEMENT", &assignment_block);
	hashmap_insert(ir_visitor, "VARIABLE_DECLARATION_STATEMENT", &decl_block);
	hashmap_insert(ir_visitor, "IF_STATEMENT", &if_block);
	hashmap_insert(ir_visitor, "IF_ELSE_STATEMENT", &if_else_block);
	hashmap_insert(ir_visitor, "WHILE_STATEMENT", &while_block);
	hashmap_insert(ir_visitor, "DO_WHILE_STATEMENT", &do_while_block);
	hashmap_insert(ir_visitor, "FOR_STATEMENT", &for_block);
	hashmap_insert(ir_visitor, "FOR_CHANGE_STATEMENT", &for_change_block);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_STATEMENT", &function_block);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_RETURN_STATEMENT", &function_block);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", &function_block);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", &function_block);
	hashmap_insert(ir_visitor, "RETURN_STATEMENT", &return_block);
    hashmap_insert(ir_visitor, "break" /* RETURN_NONE_STATEMENT */, &return_block);
    hashmap_insert(ir_visitor, "FUNCTION_CALL_WITH_NOTHING_STATEMENT", &call_block);
    hashmap_insert(ir_visitor, "FUNCTION_CALL_STATEMENT", &call_block);
    hashmap_insert(ir_visitor, "FUNCTION_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "WHILE_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "IF_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "FOR_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "BLOCK", &block_block);
    hashmap_insert(ir_visitor, "PRINT_STATEMENT", &print_block);
	hashmap_insert(ir_visitor, "GET_DECLARE_STATEMENT", &get_decl_block);
	hashmap_insert(ir_visitor, "GET_STATEMENT", &get_block);
    hashmap_insert(ir_visitor, "PRINT_INT_EXPRESSION", &print_int_block);


}


const char* irDataTypeToString(IR_DataType type) {
    switch (type) {
    case IR_NULL:           return "IR_NULL";
    case IR_TOKEN:          return "IR_TOKEN";
    case IR_BLOCK_ID:       return "IR_BLOCK_ID";
    case IR_TEMPORARY_ID:   return "IR_TEMPORARY_ID";
    case IR_TOKEN_LIST:     return "IR_TOKEN_LIST";
    case IR_STR:            return "IR_STR";
    case IR_INT:            return "IR_INT";
    default:                return "UNKNOWN";
    }
}
void printIRValuePointer(IR_Value* val);


void printIRValue(IR_Value val) {
    printf("[%s: ", irDataTypeToString(val.type));
    switch (val.type) {
    case IR_NULL:
        printf("null");
        break;
    case IR_TOKEN:
        printf("%d:%s", val.data.token.type, val.data.token.lexeme);
        break;
    case IR_BLOCK_ID:
        printf("%d", val.data.num);
        break;
    case IR_TEMPORARY_ID:
        printf("%d", val.data.num);
        break;
    case IR_TOKEN_LIST:
        printf("TOKEN_LIST   ");
        arraylist_print(val.data.values_list, printIRValuePointer);
        break;
    case IR_STR:
        printf("%s", val.data.str);
        break;
    case IR_INT:
        printf("%d", val.data.num);
        break;
    default:
        printf("UNKNOWN");
        break;
    }
    printf("]");
}

void printIRValuePointer(IR_Value* val) {
    printIRValue(*val);
}

const char* opcodeToString(IR_Opcode op) {
    switch (op) {
    case IR_RAW_STRING:    return "RAW_STRING";
    case IR_DECLARE_GLOBAL:       return "DECLARE-GLOBAL";
    case IR_DECLARE_LOCAL:       return "DECLARE-LOCAL";
    case IR_ASSIGN:        return "ASSIGN";
    case IR_ADD:           return "ADD";
    case IR_SUB:           return "SUB";
    case IR_MUL:           return "MUL";
    case IR_DIV:           return "DIV";
    case IR_MOD:           return "MOD";
    case IR_LT:            return "LT";
    case IR_LE:            return "LE";
    case IR_GT:            return "GT";
    case IR_GE:            return "GE";
    case IR_EQ:            return "EQ";
    case IR_NE:            return "NE";
    case IR_AND:           return "AND";
    case IR_OR:            return "OR";
    case IR_NOT:            return "NOT";
    case IR_CBR:           return "CBR";
    case IR_FUNC_START:    return "FUNC_START";
    case IR_FUNC_END:      return "FUNC_END";
    case IR_RETURN:        return "RETURN";
    case IR_CALL:          return "CALL";
    case IR_JMP:           return "JMP";
    case IR_PRINT:         return "PRINT";
    case IR_PRINT_INT:     return "PRINT_INT";
    case IR_PARAMETER:     return "PARAMETER";
    case IR_GLOBAL_TEMP_SPACE:return "TEMP_SPACE";
    case IR_END:           return "TEMP_SPACE";
    case IR_GET_INT:       return "GET_INT";
    default:               return "UNKNOWN";
    }
}


void printInstruction(IR_Instruction* instr) {
    printf("  %s: ", opcodeToString(instr->opcode));
    
    printIRValue(instr->arg1);

    
    if (instr->arg2.type != IR_NULL) {
        printf(" | ");
        printIRValue(instr->arg2);
    }

    
    if (instr->arg3.type != IR_NULL) {
        printf(" | ");
        printIRValue(instr->arg3);
    }
	printf("Live = %s", instr->is_live ? "true" : "false");

    printf("\n");
}


void printCFG(CodeBlock* block, int* visited) {
    if (visited[block->id])
        return;
    visited[block->id] = 1;

    printf("Basic Block %d:\n", block->id);

    
    for (int i = 0; i < block->instructions->size; i++) {
        printInstruction(*(IR_Instruction**)block->instructions->array[i]);
    }

    
    printf("  Predecessors: ");
    for (int i = 0; i < block->predecessors->size; i++) {
        printf("%d ", (*(CodeBlock**)block->predecessors->array[i])->id);
    }
    printf("\n");

    
    printf("  Successors: ");
    for (int i = 0; i < block->successors->size; i++) {
        printf("%d ", (*(CodeBlock**)block->successors->array[i])->id);
    }
    printf("\n");

	printf("  Live In: ");
	hashset_print(block->live_in, printIRValuePointer);
	printf("  Live Out: ");
	hashset_print(block->live_out, printIRValuePointer);

    printf("\n\n");

    for (int i = 0; i < block->successors->size; i++) {
        printCFG((*(CodeBlock**)block->successors->array[i]), visited);
    }


}


void printFunctionCFG() {
    printf("\n--- Function CFGs ---\n");
    for (int i = 0; i < functionCFGCount; i++) {
        printf("Function %d: '%s' CFG:\n", i, functionCFGTable[i].name);
        int* visitedFunc = calloc(next_block_id, sizeof(int));
        printCFG(functionCFGTable[i].entry, visitedFunc);
        free(visitedFunc);
        printf("\n");
    }
}

void printTOKEN3(Token* token) {
    printf("T%d:L%s", token->type, token->lexeme);
}

CodeBlock* mainCFG(SyntaxTree* tree) {
    init_variable_sizes();
    number_of_blocks = 0;
    ir_visitor = createHashMap(NONTERMINAL_COUNT_DEFUALT2, string_hash, string_equals);
    init_ir_visitor();
    globalVars = linkedlist_init(sizeof(Token));
    globalStrings = linkedlist_init(sizeof(Token));

    function_exit_blocks = linkedlist_init(sizeof(CodeBlock*));
    function_entry_blocks = linkedlist_init(sizeof(CodeBlock*));
    mainBlock = createBasicBlock();
    addIRInstruction(mainBlock, createGlobalTemps());


    CodeBlock* last = buildCFG(tree, mainBlock);
    addIRInstruction(last, createEndInstruction());
    number_of_blocks = last->id + 1;

    int maxBlocks = next_block_id;

    return mainBlock;
}
