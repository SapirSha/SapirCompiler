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

#define NONTERMINAL_COUNT_DEFUALT2 100

#pragma warning(disable:4996)

static HashMap* ir_visitor = NULL;

char* int_to_string(int num) {
    char buffer[20];
    snprintf(buffer, 20, "%d", num);
    return strdup(buffer);
}

LinkedList* function_exit_blocks = NULL; 


IR_Instruction* createRawIRInstruction(const char* text) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_RAW_STRING;

    instr->arg1.type = IR_STR;
    instr->arg1.data.str = strdup(text);

    instr->arg2.type = IR_NULL;
    instr->arg3.type = IR_NULL;
    return instr;
}

IR_Instruction* createAssignInstruction(Token dest, IR_Value src) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_ASSIGN;
    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = dest;

    instr->arg2 = src;

    instr->arg3.type = IR_NULL;
    return instr;
}

IR_Instruction* createBinaryOpInstruction(IR_Opcode op, IR_Value temp, IR_Value left, IR_Value right) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;
    
    instr->arg1 = temp;

    instr->arg2 = left;

    instr->arg3 = right;
    return instr;
}

IR_Instruction* createDeclareInstruction(Token name) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_DECLARE;


    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = name;

    instr->arg2.type = IR_NULL;
    instr->arg3.type = IR_NULL;
    return instr;
}

IR_Instruction* createConditionalInstruction(IR_Opcode op, int temp_id, IR_Value left, IR_Value right) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;

    instr->arg1.type = IR_TEMPORARY_ID;
    instr->arg1.data.num = temp_id;

    instr->arg2 = left;

    instr->arg3 = right;
    return instr;
}

IR_Instruction* createConditionalBranchInstruction(IR_Value temp_id, int then_id, int after_or_else_id) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_CBR;
    instr->arg1 = temp_id;

    instr->arg2.type = IR_BLOCK_ID;
    instr->arg2.data.num = then_id;

    instr->arg3.type = IR_BLOCK_ID;
    instr->arg3.data.num = after_or_else_id;
    return instr;
}

IR_Instruction* createFunctionLimitsInstruction(IR_Opcode op, Token name) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;
    instr->arg1.type = IR_TOKEN;
    instr->arg1.data.token = name;
    
    instr->arg2.type = IR_NULL;
    instr->arg3.type = IR_NULL;
    return instr;
}

IR_Instruction* createReturnInstruction(const IR_Value ret_val, int end_id) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_RETURN;
    
    instr->arg1 = ret_val;

    instr->arg2.type = IR_BLOCK_ID;
    instr->arg2.data.num = end_id;

    instr->arg3.type = IR_NULL;
    return instr;
}

IR_Instruction* createCallInstruction(int func_start_id, ArrayList* arg_list, IR_Value dest_temp) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_CALL;

    instr->arg1.type = IR_BLOCK_ID;
    instr->arg1.data.num = func_start_id;

    instr->arg2.type = IR_TOKEN_LIST;
    instr->arg2.data.token_list = arg_list;

    instr->arg3 = dest_temp;
    return instr;
}

IR_Instruction* createJumpInstruction(int block_id) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_JMP;
    instr->arg1.type = IR_BLOCK_ID;
    instr->arg1.data.num = block_id;
    instr->arg2.type = IR_NULL;
    instr->arg3.type = IR_NULL;
    return instr;
}

static int tempCounter = 0;
static int newTempCounter() {
    return tempCounter++;
}

static IR_Value newTemp() {
    IR_Value* temporary = malloc(sizeof(IR_Value));
    temporary->type = IR_TEMPORARY_ID;
    temporary->data.num = tempCounter++;
    return *temporary;
}



BasicBlock* buildCFG(SyntaxTree* tree, BasicBlock* current);

int next_block_id = 0;
BasicBlock* createBasicBlock(void) {
    BasicBlock* block = (BasicBlock*)malloc(sizeof(BasicBlock));
    block->id = next_block_id++;
    block->instructions = arraylist_init(sizeof(IR_Instruction*), 8);
    block->successors = arraylist_init(sizeof(BasicBlock*), 4);
    block->predecessors = arraylist_init(sizeof(BasicBlock*), 4);


    return block;
}

void addIRInstruction(BasicBlock* block, IR_Instruction* instr) {
    arraylist_add(block->instructions, &instr);
}

void addPredecessor(BasicBlock* block, BasicBlock* pred) {
    arraylist_add(block->predecessors, &pred);
}

void addSuccessor(BasicBlock* block, BasicBlock* succ) {
    arraylist_add(block->successors, &succ);
    addPredecessor(succ, block);
}

typedef struct {
    char* name;
    BasicBlock* entry;
    BasicBlock* exit;
} FunctionCFGEntry;

FunctionCFGEntry* functionCFGTable = NULL;
int functionCFGCount = 0;

void addFunctionCFG(const char* name, BasicBlock* entry, BasicBlock* exit) {
    functionCFGTable = realloc(functionCFGTable, sizeof(FunctionCFGEntry) * (functionCFGCount + 1));
    functionCFGTable[functionCFGCount].name = strdup(name);
    functionCFGTable[functionCFGCount].entry = entry;
    functionCFGTable[functionCFGCount].exit = exit;
    functionCFGCount++;
}

int find_function_index(const char* name) {
    for (int i = 0; i < functionCFGCount; i++) {
        if (strcmp(name, functionCFGTable[i].name) == 0)
            return i;
    }
    return -1;
}

static char* ast_to_string(SyntaxTree* tree) {
    if (!tree)
        return strdup("");
    if (tree->type == TERMINAL_TYPE) {
        return strdup(tree->info.terminal_info.token.lexeme);
    }
    else {
        char buffer[1024] = "";
        for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
            char* childStr = ast_to_string(tree->info.nonterminal_info.children[i]);
            strcat(buffer, childStr);
            if (i < tree->info.nonterminal_info.num_of_children - 1)
                strcat(buffer, " ");
            free(childStr);
        }
        return strdup(buffer);
    }
}

IR_Opcode mapComparisonOperator(const char* opStr) {
    if (strcmp(opStr, "<") == 0) return IR_LT;
    if (strcmp(opStr, "<=") == 0) return IR_LE;
    if (strcmp(opStr, ">") == 0) return IR_GT;
    if (strcmp(opStr, ">=") == 0) return IR_GE;
    if (strcmp(opStr, "==") == 0) return IR_EQ;
    if (strcmp(opStr, "!=") == 0) return IR_NE;
    if (strcmp(opStr, "||") == 0) return IR_OR;
    if (strcmp(opStr, "&&") == 0) return IR_AND;
    if (strcmp(opStr, "+") == 0) return IR_ADD;
    if (strcmp(opStr, "-") == 0) return IR_SUB;
    if (strcmp(opStr, "*") == 0) return IR_MUL;
    if (strcmp(opStr, "/") == 0) return IR_DIV;
    if (strcmp(opStr, "%") == 0) return IR_MOD;
    return IR_RAW_STRING;
}

IR_Value lowerExpression(SyntaxTree* exprTree, BasicBlock** current);

void find_arguments_for_call(SyntaxTree* tree, BasicBlock** current, ArrayList* list) {
    if (strcmp(tree->info.nonterminal_info.nonterminal, "ARGUMENT_LIST") == 0) {
        find_arguments_for_call(tree->info.nonterminal_info.children[0], current, list);

        IR_Value arg = lowerExpression(tree->info.nonterminal_info.children[2], current);

        arraylist_add(list, &arg);
        return;
    }

    IR_Value arg = lowerExpression(tree, current);

    arraylist_add(list, &arg);
}

IR_Value lowerFunctionCall(SyntaxTree* exprTree, BasicBlock** current) {
    char* funcName = ast_to_string(exprTree->info.nonterminal_info.children[1]);
    int found = find_function_index(funcName);

    ArrayList* call_arguments = arraylist_init(sizeof(IR_Value), 3);

    if (exprTree->info.nonterminal_info.num_of_children > 2) {
        find_arguments_for_call(exprTree->info.nonterminal_info.children[3], current, call_arguments);
    }

    IR_Value temp = newTemp();




    addIRInstruction(*current, createCallInstruction(
        functionCFGTable[found].entry->id,
        call_arguments,
        temp
    ));

    free(funcName);

    addSuccessor(*current, functionCFGTable[found].entry);
    BasicBlock* cont = createBasicBlock();
    addSuccessor(functionCFGTable[found].exit, cont);

    *current = cont;

    return temp;
}

IR_Value lowerExpression(SyntaxTree* exprTree, BasicBlock** current) {
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

        char* opStr = ast_to_string(opChild);
        IR_Opcode opCode = mapComparisonOperator(opStr);
        free(opStr);

        IR_Value temp = newTemp();

        IR_Instruction* binInstr = createBinaryOpInstruction(opCode, temp, leftValue, rightValue);
        addIRInstruction(*current, binInstr);

        return temp;
    }

    printf("EXPRESSION ERROR!");
    exit(-7);
}


BasicBlock* return_block(SyntaxTree* tree, BasicBlock* current) {
    IR_Value retVal;
    if (tree->info.nonterminal_info.num_of_children > 1) {
        retVal = lowerExpression(tree->info.nonterminal_info.children[1], &current);
    }
    else {
        retVal.type = IR_NULL;
    }
    BasicBlock* exitBlock = *(BasicBlock**)(linkedlist_peek(function_exit_blocks));
    IR_Instruction* retInstr = createReturnInstruction(retVal, exitBlock->id);
    addIRInstruction(current, retInstr);

    addSuccessor(current, exitBlock);
    return current;
}

BasicBlock* assignment_block(SyntaxTree* tree, BasicBlock* current) {
    Token dest = tree->info.nonterminal_info.children[0]->info.terminal_info.token;
    IR_Value exprResult = lowerExpression(tree->info.nonterminal_info.children[2], &current);

    IR_Instruction* assignInstr = createAssignInstruction(dest, exprResult);
    addIRInstruction(current, assignInstr);

    return current;
}

BasicBlock* decl_block(SyntaxTree* tree, BasicBlock* current) {
    Token name = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createDeclareInstruction(name));
    return current;
}

BasicBlock* decl_assignment_block(SyntaxTree* tree, BasicBlock* current) {
    Token name = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    addIRInstruction(current, createDeclareInstruction(name));

    IR_Value exprResult = lowerExpression(tree->info.nonterminal_info.children[3], &current);
    IR_Instruction* assignInstr = createAssignInstruction(name, exprResult);
    addIRInstruction(current, assignInstr);

    return current;
}



FunctionCFGEntry* buildFunctionCFG(SyntaxTree* tree) {
    Token funcName = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
    BasicBlock* entry = createBasicBlock();
    addIRInstruction(entry, createFunctionLimitsInstruction(IR_FUNC_START, funcName));

    BasicBlock* exit = createBasicBlock();
    linkedlist_push(function_exit_blocks, &exit);
    addFunctionCFG(funcName.lexeme, entry, exit);

    BasicBlock* bodyEnd = buildCFG(tree->info.nonterminal_info.children[tree->info.nonterminal_info.num_of_children - 1], entry);
    addSuccessor(bodyEnd, exit);

    addIRInstruction(exit, createFunctionLimitsInstruction(IR_FUNC_END, funcName));
    linkedlist_pop(function_exit_blocks);

    FunctionCFGEntry* fc = (FunctionCFGEntry*)malloc(sizeof(FunctionCFGEntry));
    fc->name = strdup(funcName.lexeme);
    fc->entry = entry;
    fc->exit = exit;
    return fc;
}

BasicBlock* if_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* condBlock = createBasicBlock();

    printf("CURRENT -- %d \n", current->id);
    addSuccessor(current, condBlock);
    addIRInstruction(current, createJumpInstruction(condBlock->id));

    BasicBlock* thenBlock = createBasicBlock();
    BasicBlock* then_start = thenBlock;
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);

    BasicBlock* mergeBlock = createBasicBlock();

    addSuccessor(condBlock, then_start);
    addSuccessor(condBlock, mergeBlock);
    addSuccessor(thenBlock, mergeBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &condBlock);

    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        then_start->id,
        mergeBlock->id);

    addIRInstruction(condBlock, cbrInstr);

    addIRInstruction(thenBlock, createJumpInstruction(mergeBlock->id));

    return mergeBlock;
}

BasicBlock* if_else_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* condBlock = createBasicBlock();
    addSuccessor(current, condBlock);
    addIRInstruction(current, createJumpInstruction(condBlock->id));

    BasicBlock* thenBlock = createBasicBlock();
    BasicBlock* then_start = thenBlock;

    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);
    BasicBlock* elseBlock = createBasicBlock();
    BasicBlock* else_start = elseBlock;

    elseBlock = buildCFG(tree->info.nonterminal_info.children[4], elseBlock);
    BasicBlock* mergeBlock = createBasicBlock();

    addSuccessor(condBlock, then_start);
    addSuccessor(condBlock, else_start);
    addSuccessor(thenBlock, mergeBlock);
    addSuccessor(elseBlock, mergeBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &condBlock);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        then_start->id,
        else_start->id);
    addIRInstruction(condBlock, cbrInstr);

    addIRInstruction(thenBlock, createJumpInstruction(mergeBlock->id));
    addIRInstruction(elseBlock, createJumpInstruction(mergeBlock->id));

    return mergeBlock;
}

BasicBlock* while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));


    BasicBlock* loopBody = createBasicBlock();
    BasicBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[2], loopBody);

    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

BasicBlock* do_while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopBody = createBasicBlock();
    BasicBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[1], loopBody);
    addSuccessor(current, body_start);

    addIRInstruction(current, createJumpInstruction(body_start->id));

    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(loopBody, loopHeader);
    addSuccessor(loopHeader, body_start);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

BasicBlock* block_block(SyntaxTree* tree, BasicBlock* current) {
    if (tree->info.nonterminal_info.num_of_children > 1)
        return buildCFG(tree->info.nonterminal_info.children[1], current);

    return current;
}

BasicBlock* for_block(SyntaxTree* tree, BasicBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));

    BasicBlock* loopBody = createBasicBlock();
    BasicBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    IR_Value condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        body_start->id,
        exitBlock->id);
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(loopHeader->id));

    return exitBlock;
}

BasicBlock* for_change_block(SyntaxTree* tree, BasicBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(loopHeader->id));

    BasicBlock* loopBody = createBasicBlock();
    BasicBlock* body_start = loopBody;

    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    BasicBlock* changeBlock = createBasicBlock();
    changeBlock = buildCFG(tree->info.nonterminal_info.children[6], changeBlock);
    addSuccessor(loopHeader, body_start);
    addSuccessor(loopBody, changeBlock);
    addSuccessor(changeBlock, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
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

BasicBlock* function_block(SyntaxTree* tree, BasicBlock* current) {
    FunctionCFGEntry* fc = buildFunctionCFG(tree);
    free(fc->name);
    free(fc);
    return current;
}

BasicBlock* call_block(SyntaxTree* tree, BasicBlock* current) {
    lowerFunctionCall(tree, &current);
    return current;
}

BasicBlock* statements_block(SyntaxTree* tree, BasicBlock* current) {
    for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
        current = buildCFG(tree->info.nonterminal_info.children[i], current);
    }
    return current;
}


BasicBlock* defualt_block(SyntaxTree* tree, BasicBlock* current) {
    printf("UNTARGETED NONTERMINAL: %s\n", tree->info.nonterminal_info.nonterminal);
    for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
        current = buildCFG(tree->info.nonterminal_info.children[i], current);
    }
    return current;
}

BasicBlock* (*get_block_fun(char* str))(SyntaxTree*, BasicBlock*) {
    BasicBlock* (*pointer)(SyntaxTree*, BasicBlock*) = hashmap_get(ir_visitor, str);
    if (pointer == NULL) {
        return &defualt_block;
    }
    else {
        return pointer;
    }
}

BasicBlock* buildCFG(SyntaxTree* tree, BasicBlock* current) {
    if (!tree)
        return current;

    if (tree->type == TERMINAL_TYPE) {
        printf("TERMINAL???");
        addIRInstruction(current, createRawIRInstruction(tree->info.terminal_info.token.lexeme));
        return current;
    }
    char* nonterminal = tree->info.nonterminal_info.nonterminal;

    return (*get_block_fun(nonterminal))(tree, current);

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
    hashmap_insert(ir_visitor, "FUNCTION_CALL_WITH_NOTHING_STATEMENT", &call_block);
    hashmap_insert(ir_visitor, "FUNCTION_CALL_STATEMENT", &call_block);
    hashmap_insert(ir_visitor, "FUNCTION_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "WHILE_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "IF_BLOCK", &block_block);
    hashmap_insert(ir_visitor, "FOR_BLOCK", &block_block);



}

// Helper to convert IR_DataType to a human-readable string.
const char* irDataTypeToString(IR_DataType type) {
    switch (type) {
    case IR_NULL:           return "IR_NULL";
    case IR_TOKEN:          return "IR_TOKEN";
    case IR_BLOCK_ID:       return "IR_BLOCK_ID";
    case IR_TEMPORARY_ID:   return "IR_TEMPORARY_ID";
    case IR_TOKEN_LIST:     return "IR_TOKEN_LIST";
    case IR_STR:            return "IR_STR";
    default:                return "UNKNOWN";
    }
}
void printIRValuePointer(IR_Value* val);

// Proper print function for IR_Value.
void printIRValue(IR_Value val) {
    printf("[%s: ", irDataTypeToString(val.type));
    switch (val.type) {
    case IR_NULL:
        printf("null");
        break;
    case IR_TOKEN:
        // Assuming your Token structure has a 'lexeme' field.
        printf("%d:%s", val.data.token.type, val.data.token.lexeme);
        break;
    case IR_BLOCK_ID:
        printf("%d", val.data.num);
        break;
    case IR_TEMPORARY_ID:
        printf("%d", val.data.num);
        break;
    case IR_TOKEN_LIST:
        // If you have an ArrayList API to iterate over, you might do something like:
        // for (int i = 0; i < arraylist_size(val.data.token_list); i++) {
        //     Token* t = arraylist_get(val.data.token_list, i);
        //     printf("%s", t ? t->lexeme : "null");
        //     if (i < arraylist_size(val.data.token_list) - 1)
        //         printf(", ");
        // }
        // For now, we'll print a placeholder.
        printf("TOKEN_LIST   ");
        arraylist_print(val.data.token_list, printIRValuePointer);
        break;
    case IR_STR:
        printf("%s", val.data.str);
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
// Helper: convert IR_Opcode to string.
const char* opcodeToString(IR_Opcode op) {
    switch (op) {
    case IR_RAW_STRING:    return "RAW_STRING";
    case IR_DECLARE:       return "DECLARE";
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
    case IR_CBR:           return "CBR";
    case IR_FUNC_START:    return "FUNC_START";
    case IR_FUNC_END:      return "FUNC_END";
    case IR_RETURN:        return "RETURN";
    case IR_CALL:          return "CALL";
    case IR_JMP:           return "JMP";
    default:               return "UNKNOWN";
    }
}

// Helper: prints an IR_Instruction.
void printInstruction(IR_Instruction* instr) {
    printf("  %s: ", opcodeToString(instr->opcode));
    // Print first argument.
    printIRValue(instr->arg1);

    // Print second argument if it is not IR_NULL.
    if (instr->arg2.type != IR_NULL) {
        printf(" | ");
        printIRValue(instr->arg2);
    }

    // Print third argument if it is not IR_NULL.
    if (instr->arg3.type != IR_NULL) {
        printf(" | ");
        printIRValue(instr->arg3);
    }
    printf("\n");
}

// The updated CFG printer which recursively prints each block.
void printCFG(BasicBlock* block, int* visited) {
    if (visited[block->id])
        return;
    visited[block->id] = 1;

    printf("Basic Block %d:\n", block->id);

    // Print instructions.
    for (int i = 0; i < block->instructions->size; i++) {
        printInstruction(*(IR_Instruction**)block->instructions->array[i]);
    }

    // Print predecessors.
    printf("  Predecessors: ");
    for (int i = 0; i < block->predecessors->size; i++) {
        printf("%d ", (*(BasicBlock**)block->predecessors->array[i])->id);
    }
    printf("\n");

    // Print successors.
    printf("  Successors: ");
    for (int i = 0; i < block->successors->size; i++) {
        printf("%d ", (*(BasicBlock**)block->successors->array[i])->id);
    }
    printf("\n\n");

    // Recurse over each successor.
    for (int i = 0; i < block->successors->size; i++) {
        printCFG((*(BasicBlock**)block->successors->array[i]), visited);
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

BasicBlock* mainCFG(SyntaxTree* tree) {
    ir_visitor = createHashMap(NONTERMINAL_COUNT_DEFUALT2, string_hash, string_equals);
    init_ir_visitor();

    function_exit_blocks = linkedlist_init(sizeof(BasicBlock*));

    mainBlock = createBasicBlock();
    buildCFG(tree, mainBlock);
    int maxBlocks = next_block_id;
    int* visited = calloc(maxBlocks, sizeof(int));
    printCFG(mainBlock, visited);
    free(visited);
    printf("-------------------------------------------------------------\n");
    printFunctionCFG();
    return mainBlock;
}
