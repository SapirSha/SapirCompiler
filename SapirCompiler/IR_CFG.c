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

#pragma warning(disable:4996)

char* int_to_string(int num) {
    char buffer[20];
    snprintf(buffer, 20, "%d", num);
    return strdup(buffer);
}

LinkedList* function_exit_blocks = NULL; 
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

typedef struct IR_Instruction {
    IR_Opcode opcode;
    char* arg1;        char* arg2;        char* result;  } IR_Instruction;

IR_Instruction* createRawIRInstruction(const char* text) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_RAW_STRING;
    instr->arg1 = strdup(text);
    instr->arg2 = NULL;
    instr->result = NULL;
    return instr;
}

IR_Instruction* createAssignInstruction(const char* dest, const char* src) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_ASSIGN;
    instr->arg1 = strdup(dest);
    instr->arg2 = strdup(src);
    instr->result = NULL;
    return instr;
}

IR_Instruction* createBinaryOpInstruction(IR_Opcode op, const char* left, const char* right, const char* dest) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;
    instr->arg1 = strdup(left);
    instr->arg2 = strdup(right);
    instr->result = strdup(dest);
    return instr;
}

IR_Instruction* createDeclareInstruction(const char* name) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_DECLARE;
    instr->arg1 = strdup(name);
    instr->arg2 = NULL;
    instr->result = NULL;
    return instr;
}

IR_Instruction* createConditionalInstruction(IR_Opcode op, const char* condTemp, const char* left, const char* right) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;
    instr->arg1 = strdup(left);
    instr->arg2 = strdup(right);
    instr->result = strdup(condTemp);
    return instr;
}

IR_Instruction* createConditionalBranchInstruction(const char* condTemp, const char* thenLabel, const char* mergeLabel) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_CBR;
    instr->arg1 = strdup(condTemp);
    instr->arg2 = strdup(thenLabel);
    instr->result = strdup(mergeLabel);
    return instr;
}

IR_Instruction* createFunctionLimitsInstruction(IR_Opcode op, const char* name) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = op;
    instr->arg1 = strdup(name);
    instr->arg2 = NULL;
    instr->result = NULL;
    return instr;
}

IR_Instruction* createReturnInstruction(const char* retVal, const char* endLabel) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_RETURN;
    instr->arg1 = strdup(retVal);
    instr->arg2 = strdup(endLabel);
    instr->result = NULL;
    return instr;
}

IR_Instruction* createCallInstruction(const char* funcStartLabel, const char* argList, const char* retTemp) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_CALL;
    instr->arg1 = strdup(funcStartLabel);
    instr->arg2 = strdup(argList);
    instr->result = strdup(retTemp);
    return instr;
}

IR_Instruction* createJumpInstruction(const char* blockLabel) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_JMP;
    instr->arg1 = strdup(blockLabel);
    instr->arg2 = NULL;
    instr->result = NULL;
    return instr;
}

static int tempCounter = 0;
char* newTemp() {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%dt", tempCounter++);
    return strdup(buffer);
}

typedef struct BasicBlock {
    int id;
    IR_Instruction** instructions;
    int inst_count;
    int inst_capacity;
    struct BasicBlock** successors;
    int succ_count;
    int succ_capacity;
    struct BasicBlock** predecessors;
    int pred_count;
    int pred_capacity;
} BasicBlock;
BasicBlock* buildCFG(SyntaxTree* tree, BasicBlock* current);
int next_block_id = 0;
BasicBlock* createBasicBlock(void) {
    BasicBlock* block = (BasicBlock*)malloc(sizeof(BasicBlock));
    block->id = next_block_id++;
    block->inst_count = 0;
    block->inst_capacity = 4;
    block->instructions = (IR_Instruction**)malloc(sizeof(IR_Instruction*) * block->inst_capacity);
    block->succ_count = 0;
    block->succ_capacity = 2;
    block->successors = (BasicBlock**)malloc(sizeof(BasicBlock*) * block->succ_capacity);
    block->pred_count = 0;
    block->pred_capacity = 2;
    block->predecessors = (BasicBlock**)malloc(sizeof(BasicBlock*) * block->pred_capacity);
    return block;
}

void addIRInstruction(BasicBlock* block, IR_Instruction* instr) {
    if (block->inst_count == block->inst_capacity) {
        block->inst_capacity *= 2;
        block->instructions = (IR_Instruction**)realloc(block->instructions, sizeof(IR_Instruction*) * block->inst_capacity);
    }
    block->instructions[block->inst_count++] = instr;
}

void addPredecessor(BasicBlock* block, BasicBlock* pred) {
    if (block->pred_count == block->pred_capacity) {
        block->pred_capacity *= 2;
        block->predecessors = (BasicBlock**)realloc(block->predecessors, sizeof(BasicBlock*) * block->pred_capacity);
    }
    block->predecessors[block->pred_count++] = pred;
}

void addSuccessor(BasicBlock* block, BasicBlock* succ) {
    if (block->succ_count == block->succ_capacity) {
        block->succ_capacity *= 2;
        block->successors = (BasicBlock**)realloc(block->successors, sizeof(BasicBlock*) * block->succ_capacity);
    }
    block->successors[block->succ_count++] = succ;
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

char* lowerFunctionCall(SyntaxTree* exprTree, BasicBlock** current) {
        char* funcName = ast_to_string(exprTree->info.nonterminal_info.children[1]);
    int found = find_function_index(funcName);

        char* argList = strdup("");
    if (exprTree->info.nonterminal_info.num_of_children > 2) {
        free(argList);
        argList = ast_to_string(exprTree->info.nonterminal_info.children[3]);
    }

        char* retTemp = newTemp();

    if (found == -1) {
        addIRInstruction(*current, createRawIRInstruction("error: function not found"));
        free(funcName);
        free(argList);
        return retTemp;     }

        addIRInstruction(*current, createCallInstruction(
        int_to_string(functionCFGTable[found].entry->id),
        argList,
        retTemp
    ));

    free(funcName);
    free(argList);

        addSuccessor(*current, functionCFGTable[found].entry);
        BasicBlock* cont = createBasicBlock();
        addSuccessor(functionCFGTable[found].exit, cont);
        *current = cont;

    return retTemp;
}

char* lowerExpression(SyntaxTree* exprTree, BasicBlock** current) {
    if (exprTree->type == TERMINAL_TYPE)
        return ast_to_string(exprTree);

        if (strcmp(exprTree->info.nonterminal_info.nonterminal, "FUNCTION_CALL_STATEMENT") == 0 ||
        strcmp(exprTree->info.nonterminal_info.nonterminal, "FUNCTION_CALL_WITH_NOTHING_STATEMENT") == 0) {
        return lowerFunctionCall(exprTree, current);
    }

    if (exprTree->info.nonterminal_info.num_of_children == 3) {
        SyntaxTree* leftChild = exprTree->info.nonterminal_info.children[0];
        SyntaxTree* opChild = exprTree->info.nonterminal_info.children[1];
        SyntaxTree* rightChild = exprTree->info.nonterminal_info.children[2];

        char* leftValue = lowerExpression(leftChild, current);
        char* rightValue = lowerExpression(rightChild, current);

        char* opStr = ast_to_string(opChild);
        IR_Opcode opCode = mapComparisonOperator(opStr);
        free(opStr);

        char* temp = newTemp();
        IR_Instruction* binInstr = createBinaryOpInstruction(opCode, leftValue, rightValue, temp);
        addIRInstruction(*current, binInstr);

        free(leftValue);
        free(rightValue);
        return temp;
    }

    return ast_to_string(exprTree);
}
BasicBlock* return_block(SyntaxTree* tree, BasicBlock* current) {
    char* retVal;
    if (tree->info.nonterminal_info.num_of_children > 1) {
        retVal = lowerExpression(tree->info.nonterminal_info.children[1], &current);
    }
    else {
        retVal = strdup("none");
    }
    BasicBlock* exitBlock = *(BasicBlock**)(linkedlist_peek(function_exit_blocks));
    IR_Instruction* retInstr = createReturnInstruction(retVal, int_to_string(exitBlock->id));
    addIRInstruction(current, retInstr);
    free(retVal);
    addSuccessor(current, exitBlock);
    return current;
}

BasicBlock* assignment_block(SyntaxTree* tree, BasicBlock* current) {
    char* dest = ast_to_string(tree->info.nonterminal_info.children[0]);
        char* exprResult = lowerExpression(tree->info.nonterminal_info.children[2], &current);
    IR_Instruction* assignInstr = createAssignInstruction(dest, exprResult);
    addIRInstruction(current, assignInstr);

    free(dest);
    free(exprResult);
    return current;
}

BasicBlock* decl_block(SyntaxTree* tree, BasicBlock* current) {
    char* name = ast_to_string(tree->info.nonterminal_info.children[1]);
    addIRInstruction(current, createDeclareInstruction(name));
    free(name);
    return current;
}

BasicBlock* decl_assignment_block(SyntaxTree* tree, BasicBlock* current) {
    char* name = ast_to_string(tree->info.nonterminal_info.children[1]);
    addIRInstruction(current, createDeclareInstruction(name));
    char* exprResult = lowerExpression(tree->info.nonterminal_info.children[3], &current);
    IR_Instruction* assignInstr = createAssignInstruction(name, exprResult);
    addIRInstruction(current, assignInstr);
    free(name);
    free(exprResult);
    return current;
}



FunctionCFGEntry* buildFunctionCFG(SyntaxTree* tree) {
    char* funcName = ast_to_string(tree->info.nonterminal_info.children[1]);
    BasicBlock* entry = createBasicBlock();
    addIRInstruction(entry, createFunctionLimitsInstruction(IR_FUNC_START, funcName));
    BasicBlock* exit = createBasicBlock();
    linkedlist_push(function_exit_blocks, &exit);
    BasicBlock* bodyEnd = buildCFG(tree->info.nonterminal_info.children[tree->info.nonterminal_info.num_of_children - 1], entry);
    addSuccessor(bodyEnd, exit);
    addIRInstruction(exit, createFunctionLimitsInstruction(IR_FUNC_END, funcName));
    linkedlist_pop(function_exit_blocks);

    FunctionCFGEntry* fc = (FunctionCFGEntry*)malloc(sizeof(FunctionCFGEntry));
    fc->name = strdup(funcName);
    fc->entry = entry;
    fc->exit = exit;
    free(funcName);
    return fc;
}

BasicBlock* if_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* condBlock = createBasicBlock();
    addSuccessor(current, condBlock);
    addIRInstruction(current, createJumpInstruction(int_to_string(condBlock->id)));

    BasicBlock* thenBlock = createBasicBlock();
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);
    BasicBlock* mergeBlock = createBasicBlock();
    addSuccessor(condBlock, thenBlock);
    addSuccessor(condBlock, mergeBlock);
    addSuccessor(thenBlock, mergeBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &condBlock);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(thenBlock->id),
        int_to_string(condBlock->id));
    free(condTemp);
    addIRInstruction(condBlock, cbrInstr);

    addIRInstruction(thenBlock, createJumpInstruction(int_to_string(mergeBlock->id)));

    return mergeBlock;
}

BasicBlock* if_else_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* condBlock = createBasicBlock();
    addSuccessor(current, condBlock);
    addIRInstruction(current, createJumpInstruction(int_to_string(condBlock->id)));

    BasicBlock* thenBlock = createBasicBlock();
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);
    BasicBlock* elseBlock = createBasicBlock();
    elseBlock = buildCFG(tree->info.nonterminal_info.children[4], elseBlock);
    BasicBlock* mergeBlock = createBasicBlock();
    addSuccessor(condBlock, thenBlock);
    addSuccessor(condBlock, elseBlock);
    addSuccessor(thenBlock, mergeBlock);
    addSuccessor(elseBlock, mergeBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &condBlock);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(thenBlock->id),
        int_to_string(elseBlock->id));
    addIRInstruction(condBlock, cbrInstr);
    free(condTemp);

    addIRInstruction(thenBlock, createJumpInstruction(int_to_string(mergeBlock->id)));
    addIRInstruction(elseBlock, createJumpInstruction(int_to_string(mergeBlock->id)));

    return mergeBlock;
}

BasicBlock* while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(int_to_string(loopHeader->id)));


    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[2], loopBody);
    addSuccessor(loopHeader, loopBody);
    addSuccessor(loopBody, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[1], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(loopBody->id),
        int_to_string(exitBlock->id));
    addIRInstruction(loopHeader, cbrInstr);
    free(condTemp);

    addIRInstruction(loopBody, createJumpInstruction(int_to_string(loopHeader->id)));

    return exitBlock;
}

BasicBlock* do_while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[1], loopBody);
    addSuccessor(current, loopBody);
    addIRInstruction(current, createJumpInstruction(int_to_string(loopBody->id)));

    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(loopBody, loopHeader);
    addSuccessor(loopHeader, loopBody);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(loopBody->id),
        int_to_string(exitBlock->id));
    addIRInstruction(loopHeader, cbrInstr);

    addIRInstruction(loopBody, createJumpInstruction(int_to_string(loopHeader->id)));

    free(condTemp);
    return exitBlock;
}

BasicBlock* block_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* block = createBasicBlock();
    if (tree->info.nonterminal_info.num_of_children > 1)
        return buildCFG(tree->info.nonterminal_info.children[1], block);
    return block;
}

BasicBlock* for_block(SyntaxTree* tree, BasicBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(int_to_string(loopHeader->id)));

    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    addSuccessor(loopHeader, loopBody);
    addSuccessor(loopBody, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(loopBody->id),
        int_to_string(exitBlock->id));
    addIRInstruction(loopHeader, cbrInstr);
    free(condTemp);

    addIRInstruction(loopBody, createJumpInstruction(int_to_string(loopHeader->id)));

    return exitBlock;
}

BasicBlock* for_change_block(SyntaxTree* tree, BasicBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current);
    BasicBlock* loopHeader = createBasicBlock();
    addSuccessor(current, loopHeader);
    addIRInstruction(current, createJumpInstruction(int_to_string(loopHeader->id)));

    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    BasicBlock* changeBlock = createBasicBlock();
    changeBlock = buildCFG(tree->info.nonterminal_info.children[6], changeBlock);
    addSuccessor(loopHeader, loopBody);
    addSuccessor(loopBody, changeBlock);
    addSuccessor(changeBlock, loopHeader);
    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);

    char* condTemp = lowerExpression(tree->info.nonterminal_info.children[3], &loopHeader);
    IR_Instruction* cbrInstr = createConditionalBranchInstruction(condTemp,
        int_to_string(loopBody->id),
        int_to_string(exitBlock->id));
    addIRInstruction(loopHeader, cbrInstr);
    free(condTemp);

    addIRInstruction(loopBody, createJumpInstruction(int_to_string(changeBlock->id)));
    addIRInstruction(changeBlock, createJumpInstruction(int_to_string(loopHeader->id)));

    return exitBlock;
}

BasicBlock* function_block(SyntaxTree* tree, BasicBlock* current) {
    FunctionCFGEntry* fc = buildFunctionCFG(tree);
    addFunctionCFG(fc->name, fc->entry, fc->exit);
    free(fc->name);
    free(fc);
    return current;
}

BasicBlock* call_block(SyntaxTree* tree, BasicBlock* current) {
    char* funcName = ast_to_string(tree->info.nonterminal_info.children[1]);
    int found = find_function_index(funcName);
    if (found == -1) {
        char* callStr = ast_to_string(tree);
        addIRInstruction(current, createRawIRInstruction(callStr));
        free(callStr);
        free(funcName);
        return current;
    }
    char* argList = strdup("");
    if (tree->info.nonterminal_info.num_of_children > 2) {
        argList = ast_to_string(tree->info.nonterminal_info.children[3]);
    }
        
    char* retTemp = newTemp();
    IR_Instruction* callInstr = createCallInstruction(int_to_string(functionCFGTable[found].entry->id),
        argList,
        retTemp);
    addIRInstruction(current, callInstr);
    free(funcName);
    free(argList);
    BasicBlock* cont = createBasicBlock();
    addSuccessor(current, functionCFGTable[found].entry);
    addSuccessor(functionCFGTable[found].exit, cont);
    return cont;
}

BasicBlock* statements_block(SyntaxTree* tree, BasicBlock* current) {
    for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
        current = buildCFG(tree->info.nonterminal_info.children[i], current);
    }
    return current;
}

BasicBlock* buildCFG(SyntaxTree* tree, BasicBlock* current) {
    if (!tree)
        return current;
    if (tree->type == TERMINAL_TYPE) {
        addIRInstruction(current, createRawIRInstruction(tree->info.terminal_info.token.lexeme));
        return current;
    }
    char* nonterminal = tree->info.nonterminal_info.nonterminal;
    if (strcmp(nonterminal, "STATEMENTS") == 0) {
        return statements_block(tree, current);
    }
    else if (strcmp(nonterminal, "IF_STATEMENT") == 0) {
        return if_block(tree, current);
    }
    else if (strcmp(nonterminal, "IF_ELSE_STATEMENT") == 0) {
        return if_else_block(tree, current);
    }
    else if (strcmp(nonterminal, "WHILE_STATEMENT") == 0) {
        return while_block(tree, current);
    }
    else if (strcmp(nonterminal, "DO_WHILE_STATEMENT") == 0) {
        return do_while_block(tree, current);
    }
    else if (strcmp(nonterminal, "BLOCK") == 0) {
        return statements_block(tree, current);
    }
    else if (strcmp(nonterminal, "FOR_STATEMENT") == 0) {
        return for_block(tree, current);
    }
    else if (strcmp(nonterminal, "FOR_CHANGE_STATEMENT") == 0) {
        return for_change_block(tree, current);
    }
    else if (strcmp(nonterminal, "FUNCTION_DECLARATION_STATEMENT") == 0 ||
        strcmp(nonterminal, "FUNCTION_DECLARATION_NO_RETURN_STATEMENT") == 0 ||
        strcmp(nonterminal, "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT") == 0 ||
        strcmp(nonterminal, "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT") == 0) {
        return function_block(tree, current);
    }
    else if (strcmp(nonterminal, "FUNCTION_CALL_STATEMENT") == 0 ||
        strcmp(nonterminal, "FUNCTION_CALL_WITH_NOTHING_STATEMENT") == 0) {
        return call_block(tree, current);
    }
    else if (strcmp(nonterminal, "RETURN_STATEMENT") == 0) {
        return return_block(tree, current);
    }
    if (strcmp(nonterminal, "VARIABLE_ASSIGNMENT_STATEMENT") == 0) {
        return assignment_block(tree, current);
    }
    else if (strcmp(nonterminal, "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT") == 0) {
        return decl_assignment_block(tree, current);
    }
    else if (strcmp(nonterminal, "VARIABLE_DECLARATION_STATEMENT") == 0) {
        return decl_block(tree, current);
    }
    else {
        printf("UNTARGETED NONTERMINAL: %s\n", nonterminal);
        for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
            current = buildCFG(tree->info.nonterminal_info.children[i], current);
        }
        return current;
    }
}

void printCFG(BasicBlock* block, int* visited) {
    if (visited[block->id])
        return;
    visited[block->id] = 1;
    printf("Block %d:\n", block->id);
    for (int i = 0; i < block->inst_count; i++) {
        IR_Instruction* instr = block->instructions[i];
        switch (instr->opcode) {
        case IR_RAW_STRING:
            printf("  RAW: %s\n", instr->arg1);
            break;
        case IR_ASSIGN:
            printf("  ASSIGN: %s = %s\n", instr->arg1, instr->arg2);
            break;
        case IR_ADD:
            printf("  ADD: %s + %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_SUB:
            printf("  SUB: %s - %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_MUL:
            printf("  MUL: %s * %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_DIV:
            printf("  DIV: %s / %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_MOD:
            printf("  MOD: %s %% %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_DECLARE:
            printf("  DECLARE: %s\n", instr->arg1);
            break;
        case IR_CBR:
            printf("  CBR: if %s != 0 then goto BLOCK %s else goto BLOCK %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_NE:
            printf("  NE: %s != %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_GT:
            printf("  GT: %s > %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_GE:
            printf("  GE: %s >= %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_LT:
            printf("  LT: %s < %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_LE:
            printf("  LE: %s <= %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_EQ:
            printf("  EQ: %s == %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_OR:
            printf("  OR: %s || %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_AND:
            printf("  AND: %s && %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_FUNC_START:
            printf("  FUNC_START: %s\n", instr->arg1);
            break;
        case IR_FUNC_END:
            printf("  FUNC_END: %s\n", instr->arg1);
            break;
        case IR_RETURN:
            printf("  RETURN: %s and go to BLOCK %s\n", instr->arg1, instr->arg2);
            break;
        case IR_CALL:
            printf("  CALL: Block %s with %s -> %s\n", instr->arg1, instr->arg2, instr->result);
            break;
        case IR_JMP:
            printf("  JUMP: To Block %s \n", instr->arg1);
            break;
        default:
            printf("  UNKNOWN OPCODE: %d\n", instr->opcode);
        }
    }
    for (int j = 0; j < block->pred_count; j++) {
        printf("  Predecessor: Block %d\n", block->predecessors[j]->id);
    }
    for (int j = 0; j < block->succ_count; j++) {
        printf("  Successor: Block %d\n", block->successors[j]->id);
    }
    printf("\n");
    for (int j = 0; j < block->succ_count; j++) {
        printCFG(block->successors[j], visited);
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

int mainCFG(SyntaxTree* tree) {
    function_exit_blocks = linkedlist_init(sizeof(BasicBlock*));
    BasicBlock* mainBlock = createBasicBlock();
    buildCFG(tree, mainBlock);
    int maxBlocks = next_block_id;
    int* visited = calloc(maxBlocks, sizeof(int));
    printCFG(mainBlock, visited);
    free(visited);
    printFunctionCFG();
    return 0;
}
