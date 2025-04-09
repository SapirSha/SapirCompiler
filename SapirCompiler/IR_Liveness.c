#include "IR_Liveness.h"
#include "IR_CFG.h"
#include "HashMap.h"
#include "HashSet.h"
#include "ArrayList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinkedList.h"
#include "Queue.h"
#include <stdbool.h>


Queue* Blocks = NULL;
static HashSet* seen = NULL;
HashSet* current_live = NULL;
HashSet* used_at_all = NULL;

void fill_liveness_queue(BasicBlock* entry) {

	if (hashset_contains(seen, &entry->id)) {
		return;
	}

	hashset_insert(seen, &entry->id);
	for (int i = 0; i < entry->successors->size; i++) {
		BasicBlock* succ = *(BasicBlock**)entry->successors->array[i];
		fill_liveness_queue(succ);
	}
	
	queue_enqueue(Blocks, &entry);
}

bool matters(IR_Value* value) {
	if (value->type == IR_TOKEN) {
		return value->data.token.type == TOKEN_IDENTIFIER;
	}
	else if (value->type == IR_TEMPORARY_ID) {
		return true;
	}
	else if (value->type == IR_BLOCK_ID) {
		return false; // what
	}
	else if (value->type == IR_STR) {
		return true; // another what?
	}
	return false;
}

#define insert_ir_value_to_live(ir_value_pointer) \
    if (matters(ir_value_pointer)) {\
	hashset_insert(current_live, ir_value_pointer);  \
	hashset_insert(used_at_all, ir_value_pointer); }

void handle_instruction(BasicBlock* block, int* index_of_instr) {
	IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[*index_of_instr];
	LinkedList* variables;
	switch (instr->opcode)
	{
	case IR_RAW_STRING:
		break;
	case IR_DECLARE:
		if (hashset_contains(used_at_all, &instr->arg1)) {
			instr->is_live = true;
		}
		break;
	case IR_ASSIGN:
		if (hashset_contains(current_live, &instr->arg1)) {
			hashset_remove(current_live, &instr->arg1);
			if (matters(&instr->arg2)) {
				insert_ir_value_to_live(&instr->arg2);
			}
			instr->is_live = true;
		}
		break;
	case IR_ADD:
	case IR_SUB:
	case IR_MUL:
	case IR_DIV:
	case IR_MOD:
	case IR_LT:
	case IR_LE:
	case IR_GT:
	case IR_GE:
	case IR_EQ:
	case IR_NE:
	case IR_AND:
	case IR_OR:
		// if live
		if (hashset_contains(current_live, &instr->arg1)) {
			// remove assign
			hashset_remove(current_live, &instr->arg1);
			// insert use
			if (matters(&instr->arg2)) {
				insert_ir_value_to_live(&instr->arg2);
			}
			if (matters(&instr->arg3)) {
				insert_ir_value_to_live(&instr->arg3);
			}
			instr->is_live = true;
		}
		break;
	case IR_CBR:
		if (matters(&instr->arg1)) {
			insert_ir_value_to_live(&instr->arg1);
		}
		instr->is_live = true;
		break;
	case IR_FUNC_START:

		instr->is_live = true;
		break;
	case IR_FUNC_END:

		instr->is_live = true;
		break;
	case IR_RETURN:
		if (matters(&instr->arg1)) {
			insert_ir_value_to_live(&instr->arg1);
		}
		instr->is_live = true;
		break;
	case IR_CALL:
		for (int i = 0; i < instr->arg2.data.values_list->size; i++) {
			insert_ir_value_to_live(instr->arg2.data.values_list->array[i]);
		}
		hashset_remove(current_live, &instr->arg3);

		instr->is_live = true;
		break;
	case IR_JMP:
		instr->is_live = true;
		break;
	case IR_PRINT:
		if (matters(&instr->arg1)) {
			insert_ir_value_to_live(&instr->arg1);
		}
		instr->is_live = true;
		break;
	case IR_PARAMETER:
		if (hashset_contains(used_at_all, &instr->arg1)) {
			instr->is_live = true;
		}
		hashset_remove(current_live, &instr->arg1);
		break;
	default:
		printf("WRONG %d: \n", instr->opcode);
		break;
	}

}
void remove_instruction(BasicBlock* block, int* i) {
	if (*i < 0 || *i >= block->instructions->size) {
		return;
	}

	IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[*i];
	for (int index = *i; index < block->instructions->size - 1; index++) {
		block->instructions->array[index] = block->instructions->array[index + 1];
	}
	block->instructions->size--;
	block->instructions->array[block->instructions->size] = NULL;

	(*i)--;

	free(instr);
}

void remove_dead_code(BasicBlock* block) {
	if (hashset_contains(seen, &block->id)) {
		return;
	}
	printf("Removing dead code in block %d\n", block->id);

	hashset_insert(seen, &block->id);
	for (int i = 0; i < block->instructions->size; i++) {
		IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[i];
		if (!instr->is_live) {
			printf("Removing instruction %d index %d \n", instr->opcode, i);
			remove_instruction(block, &i);
		}
	}

	for (int i = 0; i < block->successors->size; i++) {
		BasicBlock* succ = *(BasicBlock**)block->successors->array[i];
		remove_dead_code(succ);
	}
}

void liveness() {
	while (Blocks->size != 0) {
		BasicBlock* block = *(BasicBlock**)queue_dequeue(Blocks);
		printf("Block ID: %d\n", block->id);
		for (int i = 0; i < block->successors->size; i++) {
			BasicBlock* succ = *(BasicBlock**)block->successors->array[i];
			hashset_union(block->live_out, succ->live_in);
		}

		hashset_union(current_live, block->live_out);
		for (int i = block->instructions->size - 1; i >= 0; i--) {
			handle_instruction(block, &i);
		}
		bool changed = hashset_union(block->live_in, current_live);
		hashset_clear(current_live);

		if (changed) {
			for (int i = 0; i < block->predecessors->size; i++) {
				BasicBlock* pred = *(BasicBlock**)block->predecessors->array[i];
				queue_enqueue(Blocks, &pred);
			}
		}
	}
}


BasicBlock* computeLiveness(BasicBlock* entry) {
	Blocks = queue_init(sizeof(BasicBlock*));
	seen = hashset_create(32, int_hash, int_equals);
	current_live = hashset_create(32, ir_value_hash, ir_value_equals);
	used_at_all = hashset_create(32, ir_value_hash, ir_value_equals);
	
	fill_liveness_queue(entry);
	liveness();



	printf("\n\n\nBEFORE LIVENESS RESULT\n");
	int* visited = calloc(sizeof(int), 100);
	printCFG(entry, visited);
	free(visited);

	hashset_clear(seen);
	remove_dead_code(entry);

	printf("\n\n\nAFTER LIVENESS RESULT\n");
	visited = calloc(sizeof(int), 100);
	printCFG(entry, visited);
	free(visited);


	return entry;
}


