#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SyntaxTree.h"   
#include "Tokens.h"       
#include "SymbolTable.h"  
#include "Sementic.h"    
#include "IR_CFG.h"      
#include "LinkedList.h"

#pragma warning(disable:4996)

typedef enum {
    IR_RAW_STRING,      
    
} IR_Opcode;

typedef struct IR_Instruction {
    IR_Opcode opcode;
    char* arg1;
    char* arg2;
    char* result;
} IR_Instruction;

IR_Instruction* createRawIRInstruction(const char* text) {
    IR_Instruction* instr = (IR_Instruction*)malloc(sizeof(IR_Instruction));
    instr->opcode = IR_RAW_STRING;
    instr->arg1 = strdup(text);
    instr->arg2 = NULL;
    instr->result = NULL;
    return instr;
}

LinkedList* function_exit_blocks = NULL;


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





char* ast_to_string(SyntaxTree* tree) {
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



FunctionCFGEntry* buildFunctionCFG(SyntaxTree* tree) {
    
    char* funcName = ast_to_string(tree->info.nonterminal_info.children[1]);

    
    BasicBlock* entry = createBasicBlock();
    char header[256];
    snprintf(header, sizeof(header), "function %s start", funcName);
    addIRInstruction(entry, createRawIRInstruction(header));

    
    BasicBlock* exit = createBasicBlock();
    
    linkedlist_push(function_exit_blocks, &exit);

    
    BasicBlock* bodyEnd = buildCFG(tree->info.nonterminal_info.children[tree->info.nonterminal_info.num_of_children - 1], entry);
    
    addSuccessor(bodyEnd, exit);
    addIRInstruction(exit, createRawIRInstruction("function end"));

    
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
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[1]);
    addIRInstruction(condBlock, createRawIRInstruction(condStr));
    free(condStr);
    addSuccessor(current, condBlock);

    BasicBlock* thenBlock = createBasicBlock();
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);
    addSuccessor(condBlock, thenBlock);

    BasicBlock* mergeBlock = createBasicBlock();
    addSuccessor(condBlock, mergeBlock);
    addSuccessor(thenBlock, mergeBlock);
    return mergeBlock;
}

BasicBlock* if_else_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* condBlock = createBasicBlock();
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[1]);
    addIRInstruction(condBlock, createRawIRInstruction(condStr));
    free(condStr);
    addSuccessor(current, condBlock);

    BasicBlock* thenBlock = createBasicBlock();
    thenBlock = buildCFG(tree->info.nonterminal_info.children[2], thenBlock);

    BasicBlock* elseBlock = createBasicBlock();
    elseBlock = buildCFG(tree->info.nonterminal_info.children[4], elseBlock);

    addSuccessor(condBlock, thenBlock);
    addSuccessor(condBlock, elseBlock);

    BasicBlock* mergeBlock = createBasicBlock();
    addSuccessor(thenBlock, mergeBlock);
    addSuccessor(elseBlock, mergeBlock);
    return mergeBlock;
}

BasicBlock* while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopHeader = createBasicBlock();
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[1]);
    addIRInstruction(loopHeader, createRawIRInstruction(condStr));
    free(condStr);
    addSuccessor(current, loopHeader);

    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[2], loopBody);

    addSuccessor(loopHeader, loopBody); 
    addSuccessor(loopBody, loopHeader);   

    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);  
    return exitBlock;
}

BasicBlock* do_while_block(SyntaxTree* tree, BasicBlock* current) {
    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[1], loopBody);

    addSuccessor(current, loopBody);

    BasicBlock* loopHeader = createBasicBlock();
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[3]);
    addIRInstruction(loopHeader, createRawIRInstruction(condStr));
    free(condStr);


    addSuccessor(loopBody, loopHeader);
    addSuccessor(loopHeader, loopBody);

    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);
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
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[3]);
    addIRInstruction(loopHeader, createRawIRInstruction(condStr));
    free(condStr);
    addSuccessor(current, loopHeader);

    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);
    addSuccessor(loopHeader, loopBody);
    addSuccessor(loopBody, loopHeader);

    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);
    return exitBlock;
}

BasicBlock* for_change_block(SyntaxTree* tree, BasicBlock* current) {
    current = buildCFG(tree->info.nonterminal_info.children[1], current); 

    BasicBlock* loopHeader = createBasicBlock();
    char* condStr = ast_to_string(tree->info.nonterminal_info.children[3]);
    addIRInstruction(loopHeader, createRawIRInstruction(condStr));
    free(condStr);
    addSuccessor(current, loopHeader);

    BasicBlock* loopBody = createBasicBlock();
    loopBody = buildCFG(tree->info.nonterminal_info.children[4], loopBody);

    BasicBlock* changeBlock = createBasicBlock();
    changeBlock = buildCFG(tree->info.nonterminal_info.children[6], changeBlock);

    addSuccessor(loopHeader, loopBody);
    addSuccessor(loopBody, changeBlock);
    addSuccessor(changeBlock, loopHeader);

    BasicBlock* exitBlock = createBasicBlock();
    addSuccessor(loopHeader, exitBlock);
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
    char* calledFuncName = ast_to_string(tree->info.nonterminal_info.children[1]);
    int found = find_function_index(calledFuncName);
    if (found == -1) {
        char* callStr = ast_to_string(tree);
        addIRInstruction(current, createRawIRInstruction(callStr));
        free(callStr);
        free(calledFuncName);
        return current;
    }
    char* buffer = ast_to_string(tree);
    addIRInstruction(current, createRawIRInstruction(buffer));
    free(buffer);
    free(calledFuncName);

    BasicBlock* cont = createBasicBlock();
    addSuccessor(current, functionCFGTable[found].entry);
    addSuccessor(functionCFGTable[found].exit, cont);

    return cont;
}





BasicBlock* return_block(SyntaxTree* tree, BasicBlock* current) {
    
    if (tree->info.nonterminal_info.num_of_children > 1) {
        current = buildCFG(tree->info.nonterminal_info.children[1], current);
    }
    
    addIRInstruction(current, createRawIRInstruction("return"));
    
    BasicBlock* exitBlock = *(BasicBlock**)(linkedlist_peek(function_exit_blocks));
    
    addSuccessor(current, exitBlock);
    
    return exitBlock;
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
        return block_block(tree, current);
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
    else {
        printf("UNTARGETED NONTERMINAL: %s\n", nonterminal);
        addIRInstruction(current, createRawIRInstruction(nonterminal));
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
        if (instr->opcode == IR_RAW_STRING) {
            printf("  %s\n", instr->arg1);
        }
        else {
            printf("  [opcode %d] %s %s %s\n", instr->opcode,
                instr->arg1 ? instr->arg1 : "",
                instr->arg2 ? instr->arg2 : "",
                instr->result ? instr->result : "");
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
