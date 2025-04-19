#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
#include "Lexer.h"
#include "Tokens.h"
#include "Parser.h"
#include "ArrayList.h"
#include "Parser.h"
#include "SymbolTable.h"

typedef enum {
	NO_ERROR,
	LEXICAL_ERROR,
	PARSER_ERROR,
	SEMENTIC_ERROR,
	OTHER_ERROR
} ErrorType;


ErrorType current_error_state;
void out_put_error(char* msg);
void handle_unkown_character_error(char character, int row, int col);
void handle_lexical_error(LEXER_STATE previous_state, char next_input, int row, int col);
void handle_lexical_above_max_token_length(char* expected_token, int row, int col);
void handle_parser_error(int state_accured, Token* latest_token, Token* next_token, ArrayList* allowed_tokens, ArrayList* allowed_statements);
void handle_sementic_error(Token left, Data_Type left_type, Data_Type right_type);
void handle_sementic_error_identifier_already_defined(Token id);
void handle_sementic_error_identifier_not_defined(Token id);
void handle_sementic_error_condition_must_be_bool(Token id, Data_Type type);
void handle_sementic_function_doesnt_exist(Token func_call);
void handle_sementic_function_call_arguments_miscount(Token func_call, int call_amount, int func_amount);
void handle_sementic_function_call_arguments_mismatch(Token func_call, int argument_index, Data_Type arg, Data_Type param);
void handle_sementic_invalid_print_type(Token content, Data_Type type);
void handle_sementic_get_type(Token content, Data_Type type);
void handle_sementic_no_function(Token return_token);

void handle_out_of_memory_error();
void handle_other_errors(char* msg);
#endif