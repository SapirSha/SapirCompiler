#include "CodeGeneration.h"
#include "stdio.h"
#include "stdlib.h"
#include "IR_CFG.h"
#include "SymbolTable.h"
#include <string.h>
#include <stdarg.h>

#define DEFAULT_OFFSET -2
#define SIZE_OF_BOOLEAN 1

HashMap* symbol_map;
HashMap* temp_map;

char* outputcode(char* str) {
	printf("%s\n", str);
}

char* get_number_str(int num) {
	static char number[100];
	char* prefix;
	if (num >= 0)
		prefix = "+";
	else prefix = "";

	snprintf(number, 100, "%s%d", prefix, num);
	return number;
}

typedef enum {
	BX,
	CX,
	SI,
	DI,
	AVAILABLE_REG,
	// Below This Are not usable as temps
	AX,
	DX,
	SP,
	BP,
	AMOUNT_OF_REGISTERS,

	AL,
	BL,
	CL,
	DL,
} Register;


int available[AMOUNT_OF_REGISTERS];

char* get_register_name(int register_id, int size) {
	switch (size) {
	case 1:
		switch (register_id)
		{
		case AX:
			return "AL";
		case BX:
			return "BL";
		case CX:
			return "CL";
		case DX:
			return "DL";
		default:
			break;
		}
	case 2:
		switch (register_id)
		{
		case AX:
			return "AX";
		case BX:
			return "BX";
		case CX:
			return "CX";
		case DX:
			return "DX";
		default:
			break;
		}
	default:
		return "IDK";
	}

}

char* get_leftover_register(int size) {
	if (size == 1) return "AH";
	else if (size == 2) return "DX";
	else return "IDK?";
}


#define AX_REGISTER AX
#define DEFAULT_REGISTER AX


int try_to_appoint_register(int temp_id) {
	for (int i = 0; i < AVAILABLE_REG; i++) {
		if (available[i] == -1 || available[i] == temp_id){
			available[i] = temp_id;
			return i;
		}
	}
	return -1;
}

int appoint_register(int register_id, int temp_id) {
	available[register_id] = temp_id;
}


int appoint_name(int temp_id) {
	for (int i = 0; i < AMOUNT_OF_REGISTERS; i++) {
		if (available[i] == temp_id) {
			return i;
		}
	}
	return -1;
}

void free_register(int temp_id) {
	int index = appoint_name(temp_id);
	available[index] = -1;
}

char* create_label(int block_id) {
	char* label_buffer = malloc(sizeof(char) * 20);
	snprintf(label_buffer, 20, "BLOCK%d", block_id);
	return label_buffer;
}

int yes_counter = 0;
char* create_conditionl_yes_label() {
	static char yes_label[20];
	snprintf(yes_label, 20, "YES%d", yes_counter++);
	return yes_label;
}

int continue_counter = 0;
char* create_conditionl_continue_label() {
	static char con_label[20];
	snprintf(con_label, 20, "CON%d", continue_counter++);
	return con_label;
}

#define sizenamesize 32
char* size_to_size_name_data_seg(int size) {
	static char size_name[sizenamesize];
	switch (size)
	{
	case 1:
		snprintf(size_name, sizenamesize, "DB");
		break;
	case 2:
		snprintf(size_name, sizenamesize, "DW");
		break;
	case 3:
	case 4:
		snprintf(size_name, sizenamesize, "DD");
		break;
	default:
		snprintf(size_name, sizenamesize, "??");
		break;
	}
	return size_name;
}

char* size_to_size_name_pointer(int size) {
	static char size_name[sizenamesize];
	switch (size)
	{
	case 1:
		snprintf(size_name, sizenamesize, "BYTE PTR");
		break;
	case 2:
		snprintf(size_name, sizenamesize, "WORD PTR");
		break;
	case 3:
	case 4:
		snprintf(size_name, sizenamesize, "CAREFUL");
		break;
	default:
		snprintf(size_name, sizenamesize, "??");
		break;
	}
	return size_name;
}

char* get_instant_name(Token_Types tt, char* lexeme) {
	if (tt == TOKEN_TRUE) return "1";
	else if (tt == TOKEN_FALSE) return "0";
	return lexeme;
}

#define MAX_IDENTIFIER 100
int is_instant_value(IR_Value* val) {
	if (val->type == IR_TOKEN) {
		Token t = val->data.token;
		if (t.type == TOKEN_IDENTIFIER) {
		}
		else return 1;
	}
	return 0;
}

char* get_access_name(IR_Value* val) {
	char* str = malloc(sizeof(MAX_IDENTIFIER));
	if (val->type == IR_TOKEN) {
		Token t = val->data.token;
		if (t.type == TOKEN_IDENTIFIER) {
			SymbolInfo* info = hashmap_get(symbol_map, t.lexeme);
			if (info) {
				char* size_pointer = size_to_size_name_pointer(info->size);
				if (info->local) {
					snprintf(str, MAX_IDENTIFIER, "%s BP[%s] ", size_pointer, get_number_str(info->offset));
				}
				else {
					snprintf(str, MAX_IDENTIFIER, "%s", info->origin_name);
				}
			}
		}
		else {
			char* inst_name = get_instant_name(t.type, t.lexeme);
			snprintf(str, MAX_IDENTIFIER, "%s", inst_name);
		}
	}
	else if(val->type == IR_TEMPORARY_ID){
		int id = val->data.num;
		TempSymbolInfo* info = hashmap_get(temp_map, &id);

		if (appoint_name(id) == -1) {
			char* size_pointer = size_to_size_name_pointer(info->size);
			TempSymbolInfo* info = hashmap_get(temp_map, &id);
			snprintf(str, MAX_IDENTIFIER, "%s BP[%s] ", size_pointer, get_number_str(info->offset));
		}
		else {
			char* register_name = get_register_name(appoint_name(id), info->size);
			snprintf(str, MAX_IDENTIFIER, "%s", register_name);
		}
	}
	else {
		snprintf(str, MAX_IDENTIFIER, "WHAT --------------------------------------- %d", val->type);
	}

	return str;
}

void handle_global_vars() {
	Token* cur;
	static char declaration[100];
	outputcode("DATA SEGMENT");
	while (globalVars->size != 0) {
		cur = (Token*)linkedlist_pop(globalVars);
		SymbolInfo* info = hashmap_get(symbol_map, cur->lexeme);
		char* name = info->origin_name;
		char* size_name = size_to_size_name_data_seg(info->size);
		snprintf(declaration, 100, "%s %s 0", name, size_name);

		outputcode(declaration);
	}
	outputcode("DATA ENDS");
}

void handle_global_temps(BasicBlock* main) {
	static char global_temp[25];
	IR_Instruction* global_temps_instr = *(IR_Instruction**)main->instructions->array[0];
	outputcode("MOV BP, SP");
	snprintf(global_temp, 25, "SUB SP, %d", global_temps_instr->arg1.data.num);

	outputcode(global_temp);
}

#define ONEHUNDRED 100
static char main_str_buffer[ONEHUNDRED];
void handle_assign_instr(IR_Instruction* instr) {
	char* dest = get_access_name(&instr->arg1);
	char* src = get_access_name(&instr->arg2);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", dest, src);
	outputcode(main_str_buffer);
	if (instr->arg2.type == IR_TEMPORARY_ID) {
		free_register(instr->arg2.data.num);
	}

}

char* opcode_to_action[OPCODE_LENGTH] = {
	[IR_MOD] = "DIV",
	[IR_MUL] = "MUL",
	[IR_DIV] = "DIV",
	[IR_ADD] = "ADD",
	[IR_SUB] = "SUB",
	[IR_LE] = "JLE",
	[IR_LT] = "JL",
	[IR_GE] = "JGE",
	[IR_GT] = "JG",
	[IR_EQ] = "JE",
	[IR_NE] = "JNE",
	[IR_AND] = "AND",
	[IR_OR] = "OR",
};

static left_right_matters_array[OPCODE_LENGTH] = {
	[IR_DIV] = 1,
	[IR_SUB] = 1
};
bool left_right_matters(int instr_opcode) {
	return left_right_matters_array[instr_opcode] == 1;
}

void handle_add_sub_or_and_instr(IR_Instruction* instr) {
	if (
		(instr->arg2.type == IR_TEMPORARY_ID && appoint_name(instr->arg2.data.num) != -1)
		|| (instr->arg3.type == IR_TEMPORARY_ID && appoint_name(instr->arg3.data.num) != -1)
		) 
	{
		int appointment = -1;
		int register_was_left = 0;
		
		if (appoint_name(instr->arg2.data.num) != -1) {
			appointment = appoint_name(instr->arg2.data.num);
			register_was_left = 1;
		}
		else {
			appointment = appoint_name(instr->arg3.data.num);
			register_was_left = 0;
		}
		
		char* left_access = get_access_name(&instr->arg2);
		char* right_access = get_access_name(&instr->arg3);

		TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
		int size = info->size;

		char* temp_access = get_register_name(appoint_register(appointment, instr->arg1.data.num), size);


		if (!left_right_matters(instr->opcode) || register_was_left) {
			if (register_was_left == 1)
				snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], left_access, right_access);
			else 
				snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], right_access, left_access);

			outputcode(main_str_buffer);
		}
		else {
			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(DEFAULT_REGISTER, size), left_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], get_register_name(DEFAULT_REGISTER, size), right_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(DEFAULT_REGISTER, size));
			outputcode(main_str_buffer);
		}
	}
	else {
		char* left_access = get_access_name(&instr->arg2);
		char* right_access = get_access_name(&instr->arg3);
		int appointment = try_to_appoint_register(instr->arg1.data.num);
		char* temp_access = get_access_name(&instr->arg1);
		TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
		int size = info->size;


		if (appointment == -1) {
			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(DEFAULT_REGISTER, size), left_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], get_register_name(DEFAULT_REGISTER, size), right_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(DEFAULT_REGISTER, size), right_access);
			outputcode(main_str_buffer);
		}
		else {
			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(appointment, size), left_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], get_register_name(appointment, size), right_access);
			outputcode(main_str_buffer);
		}
	}
}

void handle_mul_div_instr(IR_Instruction* instr) {
	char* left_access;
	char* right_access;
	char* temp_access;

	TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
	int size = info->size;

	if (
		(instr->arg2.type == IR_TEMPORARY_ID && appoint_name(instr->arg2.data.num) != -1)
		|| (instr->arg3.type == IR_TEMPORARY_ID && appoint_name(instr->arg3.data.num) != -1)
		)
	{
		int appointment = (appoint_name(instr->arg2.data.num) != -1) ?
			appoint_name(instr->arg2.data.num) :
			appoint_name(instr->arg3.data.num)
			;

		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		temp_access = get_register_name(appoint_register(appointment, instr->arg1.data.num), size);

	}
	else {
		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		try_to_appoint_register(instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);
	}


	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX_REGISTER, size), left_access);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], right_access);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(AX_REGISTER, size));
	outputcode(main_str_buffer);

}

void handle_mod_instr(IR_Instruction* instr) {
	char* left_access;
	char* right_access;
	char* temp_access;

	TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
	int size = info->size;

	if (
		(instr->arg2.type == IR_TEMPORARY_ID && appoint_name(instr->arg2.data.num) != -1)
		|| (instr->arg3.type == IR_TEMPORARY_ID && appoint_name(instr->arg3.data.num) != -1)
		)
	{
		int appointment = (appoint_name(instr->arg2.data.num) != -1) ?
			appoint_name(instr->arg2.data.num) :
			appoint_name(instr->arg3.data.num)
			;
		if (left_right_matters(instr->opcode)) {/* In this case it doesnt no matter what */ }
		else {
			left_access = get_access_name(&instr->arg2);
			right_access = get_access_name(&instr->arg3);

			temp_access = get_register_name(appoint_register(appointment, instr->arg1.data.num), size);
		}
	}
	else {
		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		try_to_appoint_register(instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);
	}

}

void handle_condition_instr(IR_Instruction* instr) {
	char* left_access;
	char* right_access;
	char* temp_access;

	char* yes_label = create_conditionl_yes_label();
	char* con_label = create_conditionl_continue_label();


	char* jmp_condition = opcode_to_action[instr->opcode];

	TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
	int size = info->size;

	if (
		(instr->arg2.type == IR_TEMPORARY_ID && appoint_name(instr->arg2.data.num) != -1)
		|| (instr->arg3.type == IR_TEMPORARY_ID && appoint_name(instr->arg3.data.num) != -1)
		)
	{
		int appointment = (appoint_name(instr->arg2.data.num) != -1) ?
			appoint_name(instr->arg2.data.num) :
			appoint_name(instr->arg3.data.num)
			;

		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);

		temp_access = get_register_name( appoint_register(appointment, instr->arg1.data.num),size);
	}
	else {
		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		try_to_appoint_register(instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);

	}
	snprintf(main_str_buffer, ONEHUNDRED, "CMP %s, %s ", left_access, right_access);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], yes_label);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_access_name(&instr->arg1), "0");
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", con_label);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "%s:", yes_label);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_access_name(&instr->arg1), "1");
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, ONEHUNDRED, "%s:", con_label);
	outputcode(main_str_buffer);

}

void handle_branch_instr(IR_Instruction* instr) {
	char* condition_access = get_access_name(&instr->arg1);;
	if(is_instant_value(&instr->arg1)) {
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX_REGISTER, 2), "1");
		outputcode(main_str_buffer);
		condition_access = get_register_name(AX_REGISTER, 2);
	}

	char* block1 = create_label(instr->arg2.data.num);
	char* block2 = create_label(instr->arg3.data.num);

	snprintf(main_str_buffer, ONEHUNDRED, "TEST %s, %s ", condition_access, "1");
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, ONEHUNDRED, "JNZ %s ", block1);
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", block2);
	outputcode(main_str_buffer);

}

void handle_jump_instr(IR_Instruction* instr) {
	char* block_name = create_label(instr->arg1.data.num);

	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", block_name);
	outputcode(main_str_buffer);
}
void handle_end_instr(IR_Instruction* instr) {
	outputcode("MOV AX, 4C00H");
	outputcode("INT 21H");
}

void release_register(int register_id) {
	int temp_id = available[register_id];
	TempSymbolInfo* info = hashmap_get(temp_map, &temp_id);
	
	snprintf(main_str_buffer, ONEHUNDRED, "MOV BP[%s]", get_number_str(info->offset));
	outputcode(main_str_buffer);
}

void release_all_registers() {
	for (int i = 0; i < AVAILABLE_REG; i++) {
		if (available[i] != -1) {
			release_register(i);
			available[i] = -1;
			return i;
		}
	}
	return -1;
}

void handle_func_start_instr(IR_Instruction* instr) {
	release_all_registers();
	outputcode("PUSH BP");
	outputcode("MOV BP, SP");

	int reserve_space = instr->arg2.data.num;
	snprintf(main_str_buffer, 100, "SUB SP, %d",  reserve_space);
	outputcode(main_str_buffer);

}

void handle_func_end_instr(IR_Instruction* instr) {
	int reserve_space = instr->arg2.data.num;
	snprintf(main_str_buffer, 100, "ADD SP, %d", reserve_space);
	outputcode(main_str_buffer);

	outputcode("POP BP");
	int params_space = instr->arg3.data.num;
	snprintf(main_str_buffer, 100, "RET %d", params_space);
	outputcode(main_str_buffer);
}

void handle_parameter_instr(IR_Instruction* instr) {
	SymbolInfo* info = hashmap_get(symbol_map, instr->arg2.data.token.lexeme);
	char* size_pointer = size_to_size_name_pointer(info->size);
	snprintf(main_str_buffer, 100, "MOV %s, %s BP[%s]", get_register_name(AX_REGISTER, info->size), size_pointer, get_number_str( -info->offset - DEFAULT_OFFSET));
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, 100, "MOV BP[%s], %s", get_number_str(info->offset), get_register_name(AX_REGISTER, info->size));
	outputcode(main_str_buffer);
}

void handle_return_instr(IR_Instruction* instr) {
	int end_id = instr->arg2.data.num;
	char* access_ret_value = get_access_name(&instr->arg1);
	char* end_block_label = create_label(end_id);

	snprintf(main_str_buffer, 100, "MOV %s, %s", get_register_name(AX_REGISTER, 2), access_ret_value);
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, 100, "JMP %s", end_block_label);
	outputcode(main_str_buffer);
}

void handle_call_instr(IR_Instruction* instr) {
	int dest_block_id = instr->arg3.data.num;
	ArrayList* arguments = instr->arg2.data.values_list;
	int temp_dest_id = instr->arg1.data.num;

	for (int i = arguments->size - 1; i >= 0; i--) {
		IR_Value* cur = arguments->array[i];
		if (cur->type == IR_TOKEN) {
			Token t = cur->data.token;
			if (t.type == TOKEN_IDENTIFIER) {
				char* access_name = get_access_name(cur);

				SymbolInfo* info = hashmap_get(symbol_map, t.lexeme);
				if (info->size == 1) {
					outputcode("XOR AH, AH");
					snprintf(main_str_buffer, 100, "MOV AL, %s", get_register_name(AX_REGISTER,info->size), access_name);
					outputcode(main_str_buffer);
					outputcode("PUSH AX");
				}
				else if (info->size == 2) {
					snprintf(main_str_buffer, 100, "PUSH %s", access_name);
					outputcode(main_str_buffer);
				}
				else {
					exit(-920);
				}
				
			}
			else if (is_instant_value(cur)) {
				char* access_name = get_access_name(cur);
				snprintf(main_str_buffer, 100, "PUSH %s ", access_name);
				outputcode(main_str_buffer);
			}
			else {
				exit(-921);
			}
		}
		else if (cur->type == IR_TEMPORARY_ID) {
			char* access_name = get_access_name(cur);

			SymbolInfo* info = hashmap_get(temp_map, &cur->data.num);
			if (info->size == 1) {
				outputcode("XOR AH, AH");
				snprintf(main_str_buffer, 100, "MOV AL, %s", get_register_name(AX_REGISTER, info->size), access_name);
				outputcode(main_str_buffer);
				outputcode("PUSH AX");
			}
			else if (info->size == 2) {
				snprintf(main_str_buffer, 100, "PUSH %s", access_name);
				outputcode(main_str_buffer);
			}
			else {
				exit(-920);
			}
		}

	}
	

	char* function_label = create_label(dest_block_id);
	snprintf(main_str_buffer, 100, "CALL %s ", function_label);
	outputcode(main_str_buffer);

	// ASIGN TEMP, AX
	TempSymbolInfo* temp_info = hashmap_get(temp_map, &temp_dest_id);
	if (temp_info->size == 2) {
		appoint_register(AX_REGISTER, temp_dest_id);
	}
	else if (temp_info->size == 0) {
		outputcode("XOR AH, AH");
		appoint_register(AX_REGISTER, temp_dest_id);
	}
	else {
		exit(-922);
	}
}


void handle_block_instruction(IR_Instruction* instr) {
	switch (instr->opcode)
	{
	case IR_ASSIGN:
		handle_assign_instr(instr);
		break;
	case IR_ADD:
	case IR_SUB:
	case IR_AND:
	case IR_OR:
		handle_add_sub_or_and_instr(instr);
		break;
	case IR_MUL:
	case IR_DIV:
		handle_mul_div_instr(instr);
		break;
	case IR_MOD:
		handle_mod_instr(instr);
		break;
	case IR_LT:
	case IR_LE:
	case IR_GT:
	case IR_GE:
	case IR_EQ:
	case IR_NE:
		handle_condition_instr(instr);
		break;
	case IR_CBR:
		handle_branch_instr(instr);
		break;
	case IR_JMP:
		handle_jump_instr(instr);
		break;
	case IR_FUNC_START:
		handle_func_start_instr(instr);
		break;
	case IR_FUNC_END:
		handle_func_end_instr(instr);
		break;
	case IR_RETURN:
		handle_return_instr(instr);
		break;
	case IR_CALL:
		handle_call_instr(instr);
		break;
	case IR_DECLARE_GLOBAL:
	case IR_DECLARE_LOCAL:
	case IR_GLOBAL_TEMP_SPACE:
		break;
	case IR_PARAMETER:
		handle_parameter_instr(instr);
		break;
	case IR_END:
		handle_end_instr(instr);
		break;
	default:
		printf("UNKNOWN INSTR\n");
		break;
	}
}

void traverse_instructions(BasicBlock* block) {
	for (int i = 0; i < block->instructions->size; i++) {
		IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[i];
		handle_block_instruction(instr);
	}
}

void print_label_start(char* block_label) {
	snprintf(main_str_buffer, ONEHUNDRED, "%s:", block_label);
	outputcode(main_str_buffer);
}
void traverse_cfg(BasicBlock* entry, int* visitor) {
	if (visitor[entry->id] == 1) return;
	else visitor[entry->id] = 1;

	char* block_label;
	block_label = create_label(entry->id);

	print_label_start(block_label);

	traverse_instructions(entry);

	for (int i = 0; i < entry->successors->size; i++) {
		traverse_cfg(*(BasicBlock**)entry->successors->array[i], visitor);
	}
}


void init_registers() {
	for (int i = 0; i < AVAILABLE_REG; i++) {
		available[i] = -1;
	}
}

void do_the_thing_with_ds_and_ax() {
	outputcode("START:");
	outputcode("MOV AX, DATA");
	outputcode("MOV DS, AX");
}

void init_stack() {
	outputcode("STACK_SEG SEGMENT");
	outputcode("DW 1000h");
	outputcode("STACK_SEG ENDS");
}

void assume() {
	outputcode("ASSUME CS : CODE, DS : DATA, SS : STACK_SEG");
}

void generate_code(BasicBlock* entry) {
	printf("\n\n\n\n");
	symbol_map = symbol_table->SymbolMap;
	temp_map = symbol_table->TemporaryVarMap;

	init_registers();
	handle_global_vars();
	init_stack();

	outputcode("CODE SEGMENT");

	assume();
	do_the_thing_with_ds_and_ax();
	handle_global_temps(entry);


	int* visitor = calloc(100, sizeof(int));
	traverse_cfg(entry, visitor);
	free(visitor);

	outputcode("CODE ENDS");
	outputcode("END START");


}