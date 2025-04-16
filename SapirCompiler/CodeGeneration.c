#include "CodeGeneration.h"
#include "stdio.h"
#include "stdlib.h"
#include "IR_CFG.h"
#include "SymbolTable.h"
#include <string.h>
#include <stdarg.h>

#define DEFAULT_OFFSET -2
#define SIZE_OF_BOOLEAN 1

#define CONTINUE_BLOCK 0
#define FORCE_END_BLOCK 1

HashMap* symbol_map;
HashMap* temp_map;

bool used_print_num = false;
bool used_get_num = false;
bool used_print_string = false;

char* outputcode(char* str) {
	printf("%s\n", str);
}

char* get_number_str(int num) {
	static char number[32];
	char* prefix;
	if (num >= 0)
		prefix = "+";
	else prefix = "";

	snprintf(number, 32, "%s%d", prefix, num);
	return number;
}

typedef enum {
	BX,
	CX,
	DX,
	AVAILABLE_REG,
	AX,
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
			printf("REGISTER is %d?\n", register_id);
			return "IDK_REG";
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
			printf("REGISTER is %d?\n", register_id);
			return "IDK_REG";
		}
	default:
		printf("Size is %d?", size);
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




int appoint_name(int temp_id) {
	for (int i = 0; i < AVAILABLE_REG; i++) {
		if (available[i] == temp_id) {
			return i;
		}
	}
	return -1;
}

void free_register(int temp_id) {
	int index = appoint_name(temp_id);
	if (index != -1) {
		available[index] = -1;
	}
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

int get_size(IR_Value* value) {
	int size = 2;
	if (value->type == IR_TEMPORARY_ID) {
		TempSymbolInfo* info = hashmap_get(temp_map, &value->data.num);
		size = info->size;
	}
	else {
		SymbolInfo* info = hashmap_get(symbol_map, value->data.token.lexeme);
		size = info->size;
	}
	return size;
}

char* get_access_name(IR_Value* val) {
	char* str = malloc(MAX_IDENTIFIER);
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
	while (globalStrings->size != 0) {
		cur = (Token*)linkedlist_pop(globalStrings);
		StringInfo* info = hashmap_get(symbol_table->GlobalStrings, cur->lexeme);
		char* name = info->id;
		char* size_name = "DB";
		char* content = cur->lexeme;
		snprintf(declaration, 100, "GLOBAL_STRING_%d %s '%s', 0DH, 0AH, '$'", name, size_name, content);

		outputcode(declaration);
	}
	outputcode("INVALID_NUMBER_INPUT DB 0DH, 0AH, 'Input was not a number! Start Over', 0DH, 0AH, '$'");
	outputcode("NEW_LINE_CHARS DB 0DH, 0AH, '$'");
	outputcode("NUMBER_OUT_PUT_BUFFER DB 8 DUP(?)");

	outputcode("DATA ENDS");
}

void out_put_in_out_functions() {
	if (used_get_num) {
		outputcode("GETNUM PROC");
		outputcode("PUSH BX");
		outputcode("PUSH CX");
		outputcode("PUSH DX");
		outputcode("PUSH SI");
		outputcode("XOR BX, BX");
		outputcode("XOR CX, CX");
		outputcode("MOV AH, 1");
		outputcode("INT 21H");
		outputcode("CMP AL, '-'");
		outputcode("JE NEGATIVE_NUMBER_INPUT");
		outputcode("XOR SI, SI");
		outputcode("JMP POSITIVE_NUMBER_INPUT");
		outputcode("NEGATIVE_NUMBER_INPUT:");
		outputcode("INT 21H");
		outputcode("MOV SI, 1");
		outputcode("POSITIVE_NUMBER_INPUT:");
		outputcode("CMP AL, '+'");
		outputcode("JNE NUMBER_INPUT_LOOP");
		outputcode("INT 21H");
		outputcode("NUMBER_INPUT_LOOP:");
		outputcode("CMP AL, 0DH");
		outputcode("JE END_NUMBER_INPUT");
		outputcode("CMP AL, '0'");
		outputcode("JB INVALID_NUMBER_INPUT_BLOCK");
		outputcode("CMP AL, '9'");
		outputcode("JA INVALID_NUMBER_INPUT_BLOCK");
		outputcode("MOV CL, AL");
		outputcode("SUB CL, '0'");
		outputcode("MOV AX, BX");
		outputcode("MOV BX, 10");
		outputcode("MUL BX");
		outputcode("MOV BX, AX");
		outputcode("ADD BX, CX");
		outputcode("MOV AH, 1");
		outputcode("INT 21H	");
		outputcode("JMP NUMBER_INPUT_LOOP");
		outputcode("END_NUMBER_INPUT:");
		outputcode("MOV AX, BX");
		outputcode("TEST SI, 1");
		outputcode("JZ RETURN_FROM_NUMBER_INPUT");
		outputcode("MOV SI, -1");
		outputcode("IMUL SI");
		outputcode("RETURN_FROM_NUMBER_INPUT:");
		outputcode("POP BX");
		outputcode("POP CX");
		outputcode("POP DX");
		outputcode("POP SI");
		outputcode("RET");
		outputcode("INVALID_NUMBER_INPUT_BLOCK:");
		outputcode("LEA DX, INVALID_NUMBER_INPUT");
		outputcode("MOV AH, 09H");
		outputcode("INT 21H");
		outputcode("POP BX");
		outputcode("POP CX");
		outputcode("POP DX");
		outputcode("POP SI");
		outputcode("CALL GETNUM");
		outputcode("RET");
		outputcode("GETNUM ENDP");
	}

	if (used_get_num) {
		outputcode("PRINTNUM PROC");
		outputcode("PUSH AX");
		outputcode("PUSH BX");
		outputcode("PUSH CX");
		outputcode("PUSH DX");
		outputcode("PUSH SI");
		outputcode("PUSH DI");
		outputcode("LEA DI, NUMBER_OUT_PUT_BUFFER");
		outputcode("CMP AX, 8000H");
		outputcode("JB PRINT_POSITIVE_NUMBER");
		outputcode("MOV SI, WORD PTR -1");
		outputcode("IMUL SI");
		outputcode("MOV BX, AX");
		outputcode("MOV DL, '-'");
		outputcode("MOV [DI], DL");
		outputcode("INC DI");
		outputcode("JMP PRINT_NEGATIVE_NUMBER");
		outputcode("PRINT_POSITIVE_NUMBER:");
		outputcode("MOV BX, AX");
		outputcode("PRINT_NEGATIVE_NUMBER:");
		outputcode("CMP BX, 30000");
		outputcode("JAE LENGTH_MAXIMUM_NUMBER_LENGTH");
		outputcode("MOV AX, 10");
		outputcode("MOV CX, 1");
		outputcode("MOV SI, 10");
		outputcode("NUMBER_PRINT_LENGTH_LOOP:");
		outputcode("CMP AX, BX");
		outputcode("JAE END_NUMBER_PRINT_LENGTH");
		outputcode("MUL SI");
		outputcode("INC CX");
		outputcode("JMP NUMBER_PRINT_LENGTH_LOOP");
		outputcode("LENGTH_MAXIMUM_NUMBER_LENGTH:");
		outputcode("MOV CX, 5");
		outputcode("MOV SI, 10");
		outputcode("END_NUMBER_PRINT_LENGTH:");
		outputcode("MOV AX, BX");
		outputcode("ADD DI, CX");
		outputcode("MOV BX, DI");
		outputcode("DEC DI");
		outputcode("NUMBER_PRINT_LOOP:");
		outputcode("DIV SI");
		outputcode("ADD DL, '0'");
		outputcode("MOV [DI], DL");
		outputcode("DEC DI");
		outputcode("XOR DX, DX");
		outputcode("LOOP NUMBER_PRINT_LOOP");
		outputcode("END_PRINT_NUMBER:");
		outputcode("MOV AL, '$'");
		outputcode("MOV [BX], AL");
		outputcode("LEA DX, NUMBER_OUT_PUT_BUFFER");
		outputcode("MOV AH, 09H");
		outputcode("INT 21H");
		outputcode("LEA DX, NEW_LINE_CHARS");
		outputcode("MOV AH, 09H");
		outputcode("INT 21H");
		outputcode("POP AX");
		outputcode("POP BX");
		outputcode("POP CX");
		outputcode("POP DX");
		outputcode("POP SI");
		outputcode("POP DI");
		outputcode("RET");
		outputcode("PRINTNUM ENDP");
	}
	if (used_print_string) {
		outputcode("PRINTSTRING PROC");
		outputcode("PUSH DX");
		outputcode("MOV DX, AX");
		outputcode("MOV AH, 09H");
		outputcode("INT 21H");
		outputcode("POP DX");
		outputcode("RET");
		outputcode("PRINTSTRING ENDP");
	}
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

void release_register(int register_id) {
	int temp_id = available[register_id];
	if (temp_id == -1) return;

	TempSymbolInfo* info = hashmap_get(temp_map, &temp_id);

	snprintf(main_str_buffer, ONEHUNDRED, "MOV BP[%s], %s", get_number_str(info->offset), get_register_name(register_id, info->size));
	outputcode(main_str_buffer);
}

int appoint_register(int register_id, int temp_id) {
	if (available[register_id] != -1) release_register(register_id);
	available[register_id] = temp_id;
}

void handle_assign_instr(IR_Instruction* instr) {
	char* dest = get_access_name(&instr->arg1);
	char* src = get_access_name(&instr->arg2);

	if (appoint_name(dest) != -1 || appoint_name(src) != -1) {
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", dest, src);
		outputcode(main_str_buffer);

		if (instr->arg2.type == IR_TEMPORARY_ID) {
			free_register(instr->arg2.data.num);
		}
	}
	else {
		int size1 = 2;
		int size2 = 2;
		if (instr->arg1.type == IR_TEMPORARY_ID) {
			TempSymbolInfo* info = hashmap_get(temp_map, instr->arg1.data.num);
			size1 = info->size;
		}
		else {
			SymbolInfo* info = hashmap_get(symbol_map, instr->arg1.data.token.lexeme);
			size1 = info->size;
		}

		if (!is_instant_value(&instr->arg2)) {
			if (instr->arg1.type == IR_TEMPORARY_ID) {
				TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
				size1 = info->size;
			}
			else if (instr->arg1.type == IR_TOKEN) {
				SymbolInfo* info = hashmap_get(symbol_map, instr->arg1.data.token.lexeme);
				size1 = info->size;
			}
		}
		if (!is_instant_value(&instr->arg2)) {
			if (instr->arg2.type == IR_TEMPORARY_ID) {
				TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg2.data.num);
				size2 = info->size;
			}
			else if (instr->arg2.type == IR_TOKEN) {
				SymbolInfo* info = hashmap_get(symbol_map, instr->arg2.data.token.lexeme);
				size2 = info->size;
			}
		}

		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size2), src);
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", dest, get_register_name(AX, size1));
		outputcode(main_str_buffer);
	}

	free(dest);
	free(src);
}

char* opcode_to_action[OPCODE_LENGTH] = {
	[IR_MOD] = "DIV",
	[IR_MUL] = "IMUL",
	[IR_DIV] = "IDIV",
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
		
		appoint_register(appointment, instr->arg1.data.num);
		char* temp_access = get_access_name(&instr->arg1);


		if (!left_right_matters(instr->opcode) || register_was_left) {
			if (register_was_left == 1)
				snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], left_access, right_access);
			else 
				snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], right_access, left_access);

			outputcode(main_str_buffer);
		}
		else {
			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size), left_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], get_register_name(AX, size), right_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(AX, size));
			outputcode(main_str_buffer);
		}


		free(left_access);
		free(right_access);
		free(temp_access);
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

			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(DEFAULT_REGISTER, size));
			outputcode(main_str_buffer);
		}
		else {
			snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(appointment, size), left_access);
			outputcode(main_str_buffer);

			snprintf(main_str_buffer, ONEHUNDRED, "%s %s, %s ", opcode_to_action[instr->opcode], get_register_name(appointment, size), right_access);
			outputcode(main_str_buffer);
		}



		free(left_access);
		free(right_access);
		free(temp_access);
	}
}

void handle_mul_div_instr(IR_Instruction* instr) {
	char* left_access = NULL;
	char* right_access = NULL;
	char* temp_access = NULL;


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
		appoint_register(appointment, instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);
	}
	else {
		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		try_to_appoint_register(instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);
	}

	release_register(DX);

	if (is_instant_value(&instr->arg3)) {
		release_register(CX);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size), left_access);
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(CX, size), right_access);
		outputcode(main_str_buffer);
		outputcode("XOR DX, DX");
		snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], get_register_name(CX, size));
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(AX, size));
		outputcode(main_str_buffer);
	}
	else {
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size), left_access);
		outputcode(main_str_buffer);
		outputcode("XOR DX, DX");
		snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], right_access);
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_register_name(AX, size));
		outputcode(main_str_buffer);
	}

	if (left_access)
		free(left_access);
	if (right_access)
		free(right_access);
	if (temp_access)
		free(temp_access);
}

void handle_mod_instr(IR_Instruction* instr) {
	char* left_access = NULL;
	char* right_access = NULL;
	char* temp_access = NULL;

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

	release_register(DX);

	if (is_instant_value(&instr->arg3)) {
		release_register(CX);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size), left_access);
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(CX, size), right_access);
		outputcode(main_str_buffer);
		outputcode("XOR DX, DX");
		snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], get_register_name(CX, size));
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_leftover_register(size));
		outputcode(main_str_buffer);
	}
	else {
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, size), left_access);
		outputcode(main_str_buffer);
		outputcode("XOR DX, DX");
		snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], right_access);
		outputcode(main_str_buffer);
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", temp_access, get_leftover_register(size));
		outputcode(main_str_buffer);
	}
	
	if (left_access)
		free(left_access);
	if (right_access)
		free(right_access);
	if (temp_access)
		free(temp_access);
}

void handle_condition_instr(IR_Instruction* instr) {
	char* left_access = NULL;
	char* right_access = NULL;
	char* temp_access = NULL;
	int need_to_free_temp;

	char* yes_label = create_conditionl_yes_label();
	char* con_label = create_conditionl_continue_label();


	char* jmp_condition = opcode_to_action[instr->opcode];

	TempSymbolInfo* info = hashmap_get(temp_map, &instr->arg1.data.num);
	int size = info->size;
	bool need_to_free_left = true;
	int left_size = get_size(&instr->arg2);


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
		need_to_free_temp = 0;
	}
	else {
		left_access = get_access_name(&instr->arg2);
		right_access = get_access_name(&instr->arg3);
		try_to_appoint_register(instr->arg1.data.num);
		temp_access = get_access_name(&instr->arg1);
		need_to_free_temp = 1;

	}
	if ((!is_instant_value(&instr->arg2.type) &&
		(instr->arg2.type == IR_TEMPORARY_ID && appoint_name(instr->arg2.data.num) == -1 ||
		instr->arg2.type == IR_TOKEN)) &&
		(!is_instant_value(&instr->arg3.type) &&
			(instr->arg3.type == IR_TEMPORARY_ID && appoint_name(instr->arg3.data.num) == -1 ||
			instr->arg3.type == IR_TOKEN)))
	{
		
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX, left_size), left_access);
		outputcode(main_str_buffer);
		free(left_access);
		left_access = get_register_name(AX, left_size);
		need_to_free_left = false;
	}

	snprintf(main_str_buffer, ONEHUNDRED, "CMP %s, %s ", left_access, right_access);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "%s %s ", opcode_to_action[instr->opcode], yes_label);
	outputcode(main_str_buffer);
	char* access_helper = get_access_name(&instr->arg1);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", access_helper, "0");
	outputcode(main_str_buffer);
	free(access_helper);

	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", con_label);
	outputcode(main_str_buffer);
	snprintf(main_str_buffer, ONEHUNDRED, "%s:", yes_label);
	outputcode(main_str_buffer);

	access_helper = get_access_name(&instr->arg1);
	snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", access_helper, "1");
	outputcode(main_str_buffer);
	free(access_helper);

	snprintf(main_str_buffer, ONEHUNDRED, "%s:", con_label);
	outputcode(main_str_buffer);

	if (left_access && need_to_free_left)
		free(left_access);
	if (right_access)
		free(right_access);
	if (temp_access && need_to_free_temp)
		free(temp_access);
}

void handle_branch_instr(IR_Instruction* instr) {
	char* condition_access = get_access_name(&instr->arg1);
	int flag;
	if (is_instant_value(&instr->arg1)) {
		snprintf(main_str_buffer, ONEHUNDRED, "MOV %s, %s ", get_register_name(AX_REGISTER, 2), "1");
		outputcode(main_str_buffer);
		free(condition_access);
		flag = 0;
		condition_access = get_register_name(AX_REGISTER, 2);
	}
	else flag = 1;

	char* block1 = create_label(instr->arg2.data.num);
	char* block2 = create_label(instr->arg3.data.num);

	snprintf(main_str_buffer, ONEHUNDRED, "TEST %s, %s ", condition_access, "1");
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, ONEHUNDRED, "JNZ %s ", block1);
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", block2);
	outputcode(main_str_buffer);

	free(block1);
	free(block2);

	if (flag)
		free(condition_access);

}

void handle_jump_instr(IR_Instruction* instr) {
	char* block_name = create_label(instr->arg1.data.num);

	snprintf(main_str_buffer, ONEHUNDRED, "JMP %s ", block_name);
	outputcode(main_str_buffer);
	free(block_name);
}
void handle_end_instr(IR_Instruction* instr) {
	outputcode("MOV AX, 4C00H");
	outputcode("INT 21H");
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

	snprintf(main_str_buffer, 100, "MOV %s, %s", get_register_name(BX, 2), access_ret_value);
	outputcode(main_str_buffer);

	snprintf(main_str_buffer, 100, "JMP %s", end_block_label);
	outputcode(main_str_buffer);

	free(end_block_label);
	free(access_ret_value);
}

void handle_call_instr(IR_Instruction* instr) {
	int dest_block_id = instr->arg1.data.num;
	ArrayList* arguments = instr->arg2.data.values_list;
	int temp_dest_id = instr->arg3.data.num;

	for (int i = arguments->size - 1; i >= 0; i--) {
		IR_Value* cur = arguments->array[i];
		if (cur->type == IR_TOKEN) {
			Token t = cur->data.token;
			if (t.type == TOKEN_IDENTIFIER) {
				char* access_name = get_access_name(cur);
				
				SymbolInfo* info = hashmap_get(symbol_map, t.lexeme);
				if (info->size == 1) {
					outputcode("XOR AH, AH");
					snprintf(main_str_buffer, 100, "MOV %s, %s", get_register_name(AX, info->size), access_name);
					outputcode(main_str_buffer);
					outputcode("PUSH AX");
				}
				else if (info->size == 2) {
					snprintf(main_str_buffer, 100, "PUSH %s", access_name);
					outputcode(main_str_buffer);
				}
				else {
					exit(10);

				}
				free(access_name);
			}
			else if (is_instant_value(cur)) {
				char* access_name = get_access_name(cur);
				snprintf(main_str_buffer, 100, "PUSH %s ", access_name);
				outputcode(main_str_buffer);
				free(access_name);
			}
			else {
				exit(9);

			}
		}
		else if (cur->type == IR_TEMPORARY_ID) {
			char* access_name = get_access_name(cur);

			TempSymbolInfo* info = hashmap_get(temp_map, &cur->data.num);
			if (info->size == 1) {
				outputcode("XOR AH, AH");
				snprintf(main_str_buffer, 100, "MOV AL, %s", access_name);
				outputcode(main_str_buffer);
				outputcode("PUSH AX");
			}
			else if (info->size == 2) {
				snprintf(main_str_buffer, 100, "PUSH %s", access_name);
				outputcode(main_str_buffer);
			}
			else {
				printf("SIZE: %d", info->size);
					exit(8);
			}
			free(access_name);
		}

	}
	release_all_registers();

	char* function_label = create_label(dest_block_id);
	snprintf(main_str_buffer, 100, "CALL %s ", function_label);
	outputcode(main_str_buffer);
	free(function_label);


	TempSymbolInfo* temp_info = hashmap_get(temp_map, &temp_dest_id);
	snprintf(main_str_buffer, 100, "MOV BP[%s], %s", get_number_str(temp_info->offset), get_register_name(BX, temp_info->size));
	outputcode(main_str_buffer);
}

void handle_print(IR_Instruction* instr) {
	int string_id = instr->arg1.data.num;
	snprintf(main_str_buffer, 100, "LEA AX, GLOBAL_STRING_%d", string_id);
	outputcode(main_str_buffer);
	outputcode("CALL PRINTSTRING");
	used_print_string = true;
}
void handle_print_int(IR_Instruction* instr) {
	int size = 2;
	if (instr->arg1.type == IR_TEMPORARY_ID) {
		TempSymbolInfo* info = hashmap_get(temp_map, instr->arg1.data.num);
		size = info->size;
	}
	else {
		SymbolInfo* info = hashmap_get(symbol_map, instr->arg1.data.token.lexeme);
		size = info->size;
	}
	char* src_access = get_access_name(&instr->arg1);
	if (size == 1)
		outputcode("XOR AH, AH");

	snprintf(main_str_buffer, 100, "MOV %s, %s", get_register_name(AX, size), src_access);
	outputcode(main_str_buffer);
	outputcode("CALL PRINTNUM");
	free(src_access);

	used_print_num = true;
}

void handle_get_int(IR_Instruction* instr) {
	outputcode("CALL GETNUM");
	
	int size = 2;
	if (instr->arg1.type == IR_TEMPORARY_ID) {
		TempSymbolInfo* info = hashmap_get(temp_map, instr->arg1.data.num);
		size = info->size;
	}
	else {
		SymbolInfo* info = hashmap_get(symbol_map, instr->arg1.data.token.lexeme);
		size = info->size;
	}
	char* src_access = get_access_name(&instr->arg1);

	snprintf(main_str_buffer, 100, "MOV %s, %s", src_access, get_register_name(AX, size));
	outputcode(main_str_buffer);

	free(src_access);
	used_get_num = true;
}

int handle_block_instruction(IR_Instruction* instr) {
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
		return FORCE_END_BLOCK;
	case IR_FUNC_START:
		handle_func_start_instr(instr);
		break;
	case IR_FUNC_END:
		handle_func_end_instr(instr);
		break;
	case IR_RETURN:
		handle_return_instr(instr);
		return FORCE_END_BLOCK;
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
	case IR_PRINT:
		handle_print(instr);
		break; 
	case IR_PRINT_INT:
		handle_print_int(instr);
		break;
	case IR_GET_INT:
		handle_get_int(instr);
		break;
	default:
		printf("UNKNOWN INSTR\n");
		break;
	}
	return CONTINUE_BLOCK;
}

void traverse_instructions(BasicBlock* block) {
	for (int i = 0; i < block->instructions->size; i++) {
		IR_Instruction* instr = *(IR_Instruction**)block->instructions->array[i];
		int special_action = handle_block_instruction(instr);
		if (special_action == FORCE_END_BLOCK) return;
	}
}

void print_label_start(char* block_label) {
	snprintf(main_str_buffer, ONEHUNDRED, "%s:", block_label);
	outputcode(main_str_buffer);
}
void traverse_cfg(BasicBlock* entry, int* visitor) {
	if (visitor[entry->id] == 1) return;
	else visitor[entry->id] = 1;

	if (temp_map == -1)
		printf("%d --------------------<\n", entry->id);

	char* block_label;
	block_label = create_label(entry->id);

	print_label_start(block_label);

	free(block_label);
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
	outputcode("STACK_SEG SEGMENT STACK");
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

	out_put_in_out_functions();

	outputcode("CODE ENDS");
	outputcode("END START");
}