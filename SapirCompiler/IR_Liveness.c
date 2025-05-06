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
#include "ErrorHandler.h"

Queue* Blocks = NULL;
static HashSet* seen = NULL;
HashSet* current_live = NULL;
HashSet* used_at_all = NULL;

void fill_liveness_queue(CodeBlock* entry) {
	// add if not already seen
	if (hashset_contains(seen, &entry->id)) return;
	hashset_insert(seen, &entry->id); // add as seen

	// first call recursive
	for (int i = 0; i < entry->successors->size; i++) {
		CodeBlock* succ = *(CodeBlock**)entry->successors->array[i];
		fill_liveness_queue(succ); 
	}
	// then add (last first)
	queue_enqueue(Blocks, &entry);
}

bool matters(IR_Value* value) {
	if (value->type == IR_TOKEN)
		return value->data.token.type == TOKEN_IDENTIFIER;
	else if (value->type == IR_TEMPORARY_ID) 
		return true;
	else if (value->type == IR_BLOCK_ID) 
		return true;
	else if (value->type == IR_STR) 
		return true; 
	return false;
}

#define insert_ir_value_to_live(ir_value_pointer) \
    if (matters(ir_value_pointer)) {\
	hashset_insert(current_live, ir_value_pointer);  \
	hashset_insert(used_at_all, ir_value_pointer); }

static void declare(IR_Instruction* instr) {
	// if used at all, declare is live
	if (hashset_contains(used_at_all, &instr->arg1)) {
		instr->is_live = true;
	}
}

static void assign(IR_Instruction* instr) {
	// if assigned var is currently live
	if (hashset_contains(current_live, &instr->arg1)) {
		// set as dead
		hashset_remove(current_live, &instr->arg1);
		// set value as live
		if (matters(&instr->arg2)) {
			insert_ir_value_to_live(&instr->arg2);
		}

		instr->is_live = true;
	}
}

static void binary_operation(IR_Instruction* instr) {
	// if temp is live
	if (hashset_contains(current_live, &instr->arg1)) {
		// set temp as dead
		hashset_remove(current_live, &instr->arg1);
		// add other valus to live
		if (matters(&instr->arg2)) {
			insert_ir_value_to_live(&instr->arg2);
		}
		if (matters(&instr->arg3)) {
			insert_ir_value_to_live(&instr->arg3);
		}

		instr->is_live = true;
	}
}

static void condition_branch(IR_Instruction* instr) {
	// set to live always to keepy program flow
	if (matters(&instr->arg1)) {
		insert_ir_value_to_live(&instr->arg1);
	}
	instr->is_live = true;
}

static void return_l(IR_Instruction* instr) {
	// set to live always to keepy program flow
	if (matters(&instr->arg1)) {
		insert_ir_value_to_live(&instr->arg1);
	}
	instr->is_live = true;
}

static void func_call(IR_Instruction* instr) {
	// set all arguments as live
	for (int i = 0; i < instr->arg2.data.values_list->size; i++) {
		insert_ir_value_to_live(instr->arg2.data.values_list->array[i]);
	}
	// remove temp result from live
	hashset_remove(current_live, &instr->arg3);

	instr->is_live = true;
}

static void print_int(IR_Instruction* instr) {
	if (matters(&instr->arg1)) {
		// set expression as live
		insert_ir_value_to_live(&instr->arg1);
	}
	instr->is_live = true;
}

static void parameter(IR_Instruction* instr) {
	// if used at all, declare the parameter
	if (hashset_contains(used_at_all, &instr->arg2)) {
		instr->is_live = true;
	}
	// set as dead
	hashset_remove(current_live, &instr->arg2);
}

static void live(IR_Instruction* instr) {
	// defualt is live
	instr->is_live = true;
}

static void (*liveness_functions_for_instr[IR_INST_COUNT])(IR_Instruction*) = {
	[IR_DECLARE_GLOBAL] = declare,
	[IR_DECLARE_LOCAL] = declare,
	[IR_ASSIGN] = assign,
	[IR_ADD] = binary_operation,
	[IR_SUB] = binary_operation,
	[IR_MUL] = binary_operation,
	[IR_DIV] = binary_operation,
	[IR_MOD] = binary_operation,
	[IR_LT] = binary_operation,
	[IR_LE] = binary_operation,
	[IR_GT] = binary_operation,
	[IR_GE] = binary_operation,
	[IR_EQ] = binary_operation,
	[IR_NE] = binary_operation,
	[IR_AND] = binary_operation,
	[IR_OR] = binary_operation,
	[IR_CBR] = condition_branch,
	[IR_RETURN] = return_l,
	[IR_CALL] = func_call,
	[IR_FUNC_START] = live,
	[IR_FUNC_END] = live,
	[IR_JMP] = live,
	[IR_PRINT] = live,
	[IR_GLOBAL_TEMP_SPACE] = live,
	[IR_END] = live,
	[IR_PARAMETER] = parameter,
	[IR_PRINT_INT] = print_int,
	[IR_GET_INT] = live,
};


void handle_instruction(CodeBlock* block, int* index_of_instr) {
	IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[*index_of_instr];
	if (instr->opcode >= 0 && instr->opcode < IR_INST_COUNT)
		if (liveness_functions_for_instr[instr->opcode] != 0)
			liveness_functions_for_instr[instr->opcode](instr);
}

void remove_instruction(CodeBlock* block, int* i) {
	if (*i < 0 || *i >= block->instructions->size) return;

	IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[*i];
	for (int index = *i; index < block->instructions->size - 1; index++) {
		block->instructions->array[index] = block->instructions->array[index + 1];
	}
	block->instructions->size--;
	block->instructions->array[block->instructions->size] = NULL;

	(*i)--;

	free(instr);
}

void remove_dead_code(CodeBlock* block) {
	// go only trough unseen
	if (hashset_contains(seen, &block->id)) return;
	hashset_insert(seen, &block->id);

	for (int i = 0; i < block->instructions->size; i++) {
		// if an instruction was never set as live remove it
		IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[i];
		if (!instr->is_live) {
			remove_instruction(block, &i);
		}
	}

	for (int i = 0; i < block->successors->size; i++) {
		// remove dead code for successors
		CodeBlock* succ = *(CodeBlock**)block->successors->array[i];
		remove_dead_code(succ);
	}
}

void liveness() {
	while (Blocks->size != 0) { // for each block ( starting from last)
		CodeBlock* block = *(CodeBlock**)queue_dequeue(Blocks);
		// live-out(a) = union(live-in(predecessors))
		for (int i = 0; i < block->successors->size; i++) {
			CodeBlock* succ = *(CodeBlock**)block->successors->array[i];
			hashset_union(block->live_out, succ->live_in);
		}
		// current-live is empty here ( current-live = live-out
		hashset_union(current_live, block->live_out);
		for (int i = block->instructions->size - 1; i >= 0; i--) {
			handle_instruction(block, &i);
		}
		bool changed = hashset_union(block->live_in, current_live);
		hashset_clear(current_live);

		// if live-in(a) changed, check if something changed in the predecessors
		if (changed) {
			for (int i = 0; i < block->predecessors->size; i++) {
				CodeBlock* pred = *(CodeBlock**)block->predecessors->array[i];
				queue_enqueue(Blocks, &pred);
			}
		}
	}
}

CodeBlock* computeLiveness(CodeBlock* entry) {
	Blocks = queue_init(sizeof(CodeBlock*));
	seen = hashset_create(32, int_hash, int_equals);
	current_live = hashset_create(32, ir_value_hash, ir_value_equals);
	used_at_all = hashset_create(32, ir_value_hash, ir_value_equals);
	
	fill_liveness_queue(entry);
	liveness();


	hashset_clear(seen);
	remove_dead_code(entry);



	return entry;
}


