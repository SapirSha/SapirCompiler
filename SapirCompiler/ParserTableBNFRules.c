#include "ParserTableGenerator.h"

ArrayList* rules;
int associationArray[NUM_OF_TOKENS];

/*
A function that adds a rule into the rules array list, that represents the BNF
 * Nonterminals first letter is uppercase
*/
void add_rule(const char* nonterminal, const char* content) {
    Rule rule;
    rule.nonterminal = strdup(nonterminal);
    rule.ruleContent = strdup(content);
    rule.ruleTerminalCount = count_symbols(content);
    rule.ruleID = rules->size;
    arraylist_add(rules, &rule);
}

void add_rules() {
    /*
    add_rule("PROGRAM", "STATEMENTS");
    add_rule("STATEMENTS", "STATEMENTS STATEMENT");
    add_rule("STATEMENTS", "STATEMENT");

    add_rule("STATEMENT", "FUNCTION_DECLARATION");
    add_rule("STATEMENT", "VARIABLE_CHANGES");
    add_rule("STATEMENT", "IF_STATEMENTS");
    add_rule("STATEMENT", "WHILE_STATEMENTS");
    add_rule("STATEMENT", "FOR_STATEMENTS");
    add_rule("STATEMENT", "PRINT_STATEMENT");
    add_rule("STATEMENT", "GET_STATEMENTS");
    add_rule("STATEMENT", "FUNCTION_CALL_STATEMENTS");
    add_rule("STATEMENT", "RETURN_STATEMENTS");
    add_rule("STATEMENT", "BLOCK");

    add_rule("BLOCK", "{ STATEMENTS }");
    add_rule("SCOPED_BLOCK", "{ STATEMENTS }");
    add_rule("BLOCK_OR_STATEMENT", "SCOPED_BLOCK");
    add_rule("BLOCK_OR_STATEMENT", "STATEMENT");

    add_rule("VARIABLE_TYPE", "int");
    add_rule("VARIABLE_TYPE", "bool");

    add_rule("FUNCTION_BASE", "function identifier");
    add_rule("FUNCTION_GETS_BASE", "gets PARAMETER_LIST");
    add_rule("FUNCTION_RETURNS_BASE", "returns VARIABLE_TYPE");

    add_rule("PARAMETER_LIST", "PARAMETER_LIST , PARAMETER");
    add_rule("PARAMETER_LIST", "PARAMETER");
    add_rule("PARAMETER", "VARIABLE_TYPE identifier");

    add_rule("FUNCTION_DECLARATION", "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT");
    add_rule("FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", "FUNCTION_BASE SCOPED_BLOCK");

    add_rule("FUNCTION_DECLARATION", "FUNCTION_DECLARATION_NO_RETURN_STATEMENT");
    add_rule("FUNCTION_DECLARATION_NO_RETURN_STATEMENT", "FUNCTION_BASE FUNCTION_GETS_BASE SCOPED_BLOCK");

    add_rule("FUNCTION_DECLARATION", "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT");
    add_rule("FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", "FUNCTION_BASE FUNCTION_RETURNS_BASE SCOPED_BLOCK");

    add_rule("FUNCTION_DECLARATION", "FUNCTION_DECLARATION_STATEMENT");
    add_rule("FUNCTION_DECLARATION_STATEMENT", "FUNCTION_BASE FUNCTION_GETS_BASE FUNCTION_RETURNS_BASE SCOPED_BLOCK");

    add_rule("VARIABLE_CHANGES", "VARIABLE_DECLARATION_STATEMENT");
    add_rule("VARIABLE_DECLARATION_STATEMENT", "VARIABLE_TYPE identifier");

    add_rule("VARIABLE_CHANGES", "VARIABLE_ASSIGNMENT_STATEMENT");
    add_rule("VARIABLE_ASSIGNMENT_STATEMENT", "identifier = EXPRESSION");

    add_rule("VARIABLE_CHANGES", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT");
    add_rule("VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", "VARIABLE_TYPE identifier = EXPRESSION");

    add_rule("IF_STATEMENTS", "IF_ELSE_STATEMENT");
    add_rule("IF_ELSE_STATEMENT", "if EXPRESSION BLOCK_OR_STATEMENT else BLOCK_OR_STATEMENT");

    add_rule("IF_STATEMENTS", "IF_STATEMENT");
    add_rule("IF_STATEMENT", "if EXPRESSION BLOCK_OR_STATEMENT");


    add_rule("WHILE_STATEMENTS", "WHILE_STATEMENT");
    add_rule("WHILE_STATEMENT", "while EXPRESSION BLOCK_OR_STATEMENT");

    add_rule("WHILE_STATEMENTS", "DO_WHILE_STATEMENT");
    add_rule("DO_WHILE_STATEMENT", "do BLOCK_OR_STATEMENT while EXPRESSION");

    add_rule("FOR_ASSIGNMENT", "VARIABLE_ASSIGNMENT_STATEMENT");
    add_rule("FOR_ASSIGNMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT");

    add_rule("FOR_STATEMENTS", "FOR_STATEMENT");
    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while EXPRESSION BLOCK_OR_STATEMENT");

    add_rule("FOR_STATEMENTS", "FOR_CHANGE_STATEMENT");
    add_rule("FOR_CHANGE_STATEMENT", "for FOR_ASSIGNMENT while EXPRESSION BLOCK_OR_STATEMENT change VARIABLE_ASSIGNMENT_STATEMENT");

    add_rule("GET_STATEMENTS", "GET_STATEMENT");
    add_rule("GET_STATEMENT", "get identifier");

    add_rule("GET_STATEMENTS", "GET_DECLARE_STATEMENT");
    add_rule("GET_DECLARE_STATEMENT", "get VARIABLE_DECLARATION_STATEMENT");

    add_rule("PRINT_STATEMENT", "print EXPRESSION");
    add_rule("PRINT_STATEMENT", "print string_literal");

    add_rule("ARGUMENT_LIST", "ARGUMENT_LIST , EXPRESSION");
    add_rule("ARGUMENT_LIST", "EXPRESSION");

    add_rule("FUNCTION_CALL_STATEMENTS", "FUNCTION_CALL_STATEMENT");
    add_rule("FUNCTION_CALL_STATEMENT", "call identifier ( ARGUMENT_LIST )");

    add_rule("FUNCTION_CALL_STATEMENTS", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("FUNCTION_CALL_WITH_NOTHING_STATEMENT", "call identifier");
    add_rule("FUNCTION_CALL_WITH_NOTHING_STATEMENT", "call identifier ( )");

    add_rule("RETURN_STATEMENTS", "RETURN_STATEMENT");
    add_rule("RETURN_STATEMENT", "return EXPRESSION");

    add_rule("RETURN_STATEMENTS", "RETURN_NONE_STATEMENT");
    add_rule("RETURN_NONE_STATEMENT", "break");

    add_rule("CONDITION_LIST", "CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST && CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST || CONDITION");

    add_rule("CONDITION", "CONDITION_PRIORITY");
    add_rule("CONDITION", "EXPRESSION == EXPRESSION");
    add_rule("CONDITION", "EXPRESSION != EXPRESSION");
    add_rule("CONDITION", "EXPRESSION > EXPRESSION");
    add_rule("CONDITION", "EXPRESSION >= EXPRESSION");
    add_rule("CONDITION", "EXPRESSION < EXPRESSION");
    add_rule("CONDITION", "EXPRESSION <= EXPRESSION");
    add_rule("CONDITION", "FUNCTION_CALL_STATEMENT");
    add_rule("CONDITION", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("CONDITION", "true");
    add_rule("CONDITION", "false");

    add_rule("CONDITION_PRIORITY", "( CONDITION_LIST )");

    add_rule("EXPRESSION", "EXPRESSION + TERM");
    add_rule("EXPRESSION", "EXPRESSION - TERM");
    add_rule("EXPRESSION", "TERM");

    add_rule("TERM", "TERM % FACTOR");
    add_rule("TERM", "TERM * FACTOR");
    add_rule("TERM", "TERM / FACTOR");
    add_rule("TERM", "FACTOR");

    add_rule("FACTOR", "NEGATIVE_FACTOR");
    add_rule("FACTOR", "( EXPRESSION )");
    add_rule("FACTOR", "identifier");
    add_rule("FACTOR", "number");
    add_rule("FACTOR", "CONDITION_LIST");
    add_rule("FACTOR", "FUNCTION_CALL_STATEMENTS");

    add_rule("NEGATIVE_FACTOR", "- FACTOR");
    */


    add_rule("PROGRAM", "STATEMENTS"); //
    add_rule("STATEMENTS", "STATEMENTS STATEMENT");
    add_rule("STATEMENTS", "STATEMENT");
    add_rule("STATEMENTS", "STATEMENTS STATEMENT");
    add_rule("STATEMENTS", "STATEMENT"); //

    add_rule("STATEMENT", "VARIABLE_DECLARATION_STATEMENT"); //
    add_rule("STATEMENT", "VARIABLE_ASSIGNMENT_STATEMENT"); //
    add_rule("STATEMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT"); //
    add_rule("STATEMENT", "IF_STATEMENT"); //
    add_rule("STATEMENT", "IF_ELSE_STATEMENT"); //
    add_rule("STATEMENT", "WHILE_STATEMENT"); //
    add_rule("STATEMENT", "DO_WHILE_STATEMENT"); //
    add_rule("STATEMENT", "FOR_STATEMENT"); // 
    add_rule("STATEMENT", "FOR_CHANGE_STATEMENT"); //
    add_rule("STATEMENT", "PRINT_STATEMENT");
    add_rule("STATEMENT", "GET_STATEMENT");
    add_rule("STATEMENT", "GET_DECLARE_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_RETURN_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_CALL_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("STATEMENT", "RETURN_STATEMENT");
    add_rule("STATEMENT", "RETURN_NONE_STATEMENT");
    add_rule("STATEMENT", "BLOCK");

    add_rule("CONDITION_LIST", "CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST && CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST || CONDITION");

    add_rule("CONDITION", "( CONDITION_LIST )");
    add_rule("CONDITION", "EXPRESSION == EXPRESSION");
    add_rule("CONDITION", "EXPRESSION != EXPRESSION");
    add_rule("CONDITION", "EXPRESSION > EXPRESSION");
    add_rule("CONDITION", "EXPRESSION >= EXPRESSION");
    add_rule("CONDITION", "EXPRESSION < EXPRESSION");
    add_rule("CONDITION", "EXPRESSION <= EXPRESSION");
    add_rule("CONDITION", "FUNCTION_CALL_STATEMENT");
    add_rule("CONDITION", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("CONDITION", "true");
    add_rule("CONDITION", "false");

    add_rule("EXPRESSION", "EXPRESSION + TERM");
    add_rule("EXPRESSION", "EXPRESSION - TERM");
    add_rule("EXPRESSION", "TERM");

    add_rule("TERM", "TERM % FACTOR");
    add_rule("TERM", "TERM * FACTOR");
    add_rule("TERM", "TERM / FACTOR");
    add_rule("TERM", "FACTOR");

    add_rule("FACTOR", "( EXPRESSION )");
    add_rule("FACTOR", "identifier");
    add_rule("FACTOR", "number");
    add_rule("FACTOR", "CONDITION_LIST");
    add_rule("FACTOR", "FUNCTION_CALL_STATEMENT");
    add_rule("FACTOR", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");

    add_rule("VARIABLE_TYPE", "int");
    add_rule("VARIABLE_TYPE", "bool");

    add_rule("BLOCK", "{ STATEMENTS }"); //
    add_rule("BLOCK", "{ }"); //

    add_rule("IF_STATEMENT", "if CONDITION_LIST IF_BLOCK"); //
    add_rule("IF_ELSE_STATEMENT", "if CONDITION_LIST IF_BLOCK else IF_BLOCK "); //

    add_rule("IF_BLOCK", "{ STATEMENTS }"); // ------
    add_rule("IF_BLOCK", "STATEMENT"); //

    add_rule("VARIABLE_DECLARATION_STATEMENT", "VARIABLE_TYPE identifier"); //
    add_rule("VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", "VARIABLE_TYPE identifier = EXPRESSION"); //

    add_rule("VARIABLE_ASSIGNMENT_STATEMENT", "identifier = EXPRESSION"); //

    add_rule("WHILE_STATEMENT", "while CONDITION_LIST WHILE_BLOCK"); //
    add_rule("DO_WHILE_STATEMENT", "do WHILE_BLOCK while CONDITION_LIST"); //

    add_rule("WHILE_BLOCK", "{ STATEMENT }"); //
    add_rule("WHILE_BLOCK", "STATEMENT"); //


    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK"); //
    add_rule("FOR_CHANGE_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK change CHANGE_BLOCK"); //

    add_rule("FOR_ASSIGNMENT", "VARIABLE_ASSIGNMENT_STATEMENT"); //
    add_rule("FOR_ASSIGNMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT"); //

    add_rule("FOR_BLOCK", "{ STATEMENTS }"); //
    add_rule("FOR_BLOCK", "STATEMENT"); //

    add_rule("CHANGE_BLOCK", "VARIABLE_ASSIGNMENT_STATEMENT"); //

    add_rule("PRINT_STATEMENT", "print string_literal");
    add_rule("PRINT_STATEMENT", "print EXPRESSION");

    add_rule("GET_STATEMENT", "get identifier");
    add_rule("GET_DECLARE_STATEMENT", "get VARIABLE_DECLARATION_STATEMENT");

    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier gets PARAMETER_LIST returns VARIABLE_TYPE FUNCTION_BLOCK"); //
    add_rule("FUNCTION_DECLARATION_NO_RETURN_STATEMENT", "function identifier gets PARAMETER_LIST FUNCTION_BLOCK"); //
    add_rule("FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", "function identifier returns VARIABLE_TYPE FUNCTION_BLOCK");
    add_rule("FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", "function identifier FUNCTION_BLOCK"); //

    add_rule("PARAMETER_LIST", "PARAMETER_LIST , PARAMETER");
    add_rule("PARAMETER_LIST", "PARAMETER");
    add_rule("PARAMETER", "VARIABLE_TYPE identifier");

    add_rule("FUNCTION_BLOCK", "{ FUNCTION_STATEMENTS }");

    add_rule("FUNCTION_STATEMENTS", "FUNCTION_STATEMENTS FUNCTION_STATEMENT");
    add_rule("FUNCTION_STATEMENTS", "FUNCTION_STATEMENT");

    add_rule("FUNCTION_STATEMENT", "VARIABLE_DECLARATION_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "VARIABLE_ASSIGNMENT_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "IF_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "IF_ELSE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "WHILE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "DO_WHILE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "FOR_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "FOR_CHANGE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "PRINT_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "GET_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "GET_DECLARE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "FUNCTION_CALL_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "RETURN_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "RETURN_NONE_STATEMENT");
    add_rule("FUNCTION_STATEMENT", "BLOCK");

    add_rule("RETURN_STATEMENT", "return EXPRESSION");
    add_rule("RETURN_NONE_STATEMENT", "break");

    add_rule("FUNCTION_CALL_STATEMENT", "call identifier ( ARGUMENT_LIST )");
    add_rule("FUNCTION_CALL_WITH_NOTHING_STATEMENT", "call identifier");

    add_rule("ARGUMENT_LIST", "ARGUMENT_LIST , EXPRESSION");
    add_rule("ARGUMENT_LIST", "EXPRESSION");

}

void createAssociationMap() {
    associationArray[TOKEN_LBRACES] = find_column_of_terminal_in_table("{");
    associationArray[TOKEN_RBRACES] = find_column_of_terminal_in_table("}");
    associationArray[TOKEN_OPERATOR_ALSO] = find_column_of_terminal_in_table("&&");
    associationArray[TOKEN_OPERATOR_EITHER] = find_column_of_terminal_in_table("||");
    associationArray[TOKEN_LPAREN] = find_column_of_terminal_in_table("(");
    associationArray[TOKEN_RPAREN] = find_column_of_terminal_in_table(")");
    associationArray[TOKEN_OPERATOR_EQUAL] = find_column_of_terminal_in_table("==");
    associationArray[TOKEN_OPERATOR_NOT_EQUAL] = find_column_of_terminal_in_table("!=");
    associationArray[TOKEN_OPERATOR_GREATER] = find_column_of_terminal_in_table(">");
    associationArray[TOKEN_OPERATOR_GREATER_EQUAL] = find_column_of_terminal_in_table(">=");
    associationArray[TOKEN_OPERATOR_LESS] = find_column_of_terminal_in_table("<");
    associationArray[TOKEN_OPERATOR_LESS_EQUAL] = find_column_of_terminal_in_table("<=");
    associationArray[TOKEN_OPERATOR_PLUS] = find_column_of_terminal_in_table("+");
    associationArray[TOKEN_OPERATOR_MINUS] = find_column_of_terminal_in_table("-");
    associationArray[TOKEN_OPERATOR_MULTIPLY] = find_column_of_terminal_in_table("*");
    associationArray[TOKEN_OPERATOR_DIVIDE] = find_column_of_terminal_in_table("/");
    associationArray[TOKEN_IF] = find_column_of_terminal_in_table("if");
    associationArray[TOKEN_EOF] = find_column_of_terminal_in_table("$");
    associationArray[TOKEN_IDENTIFIER] = find_column_of_terminal_in_table("identifier");
    associationArray[TOKEN_NUMBER] = find_column_of_terminal_in_table("number");
    associationArray[TOKEN_INT] = find_column_of_terminal_in_table("int");
    associationArray[TOKEN_ELSE] = find_column_of_terminal_in_table("else");
    associationArray[TOKEN_OPERATOR_ASSIGN] = find_column_of_terminal_in_table("=");
    associationArray[TOKEN_DO] = find_column_of_terminal_in_table("do");
    associationArray[TOKEN_WHILE] = find_column_of_terminal_in_table("while");
    associationArray[TOKEN_FOR] = find_column_of_terminal_in_table("for");
    associationArray[TOKEN_CHANGE] = find_column_of_terminal_in_table("change");
    associationArray[TOKEN_PRINT] = find_column_of_terminal_in_table("print");
    associationArray[TOKEN_GET] = find_column_of_terminal_in_table("get");
    associationArray[TOKEN_RETURNS] = find_column_of_terminal_in_table("returns");
    associationArray[TOKEN_GETS] = find_column_of_terminal_in_table("gets");
    associationArray[TOKEN_FUNCTION] = find_column_of_terminal_in_table("function");
    associationArray[TOKEN_COMMA] = find_column_of_terminal_in_table(",");
    associationArray[TOKEN_CALL] = find_column_of_terminal_in_table("call");
    associationArray[TOKEN_RETURN] = find_column_of_terminal_in_table("return");
    associationArray[TOKEN_OPERATOR_MODULO] = find_column_of_terminal_in_table("%");
    associationArray[TOKEN_BOOL] = find_column_of_terminal_in_table("bool");
    associationArray[TOKEN_STRING_LITERAL] = find_column_of_terminal_in_table("string_literal");
    associationArray[TOKEN_TRUE] = find_column_of_terminal_in_table("true");
    associationArray[TOKEN_FALSE] = find_column_of_terminal_in_table("false");
    associationArray[TOKEN_BREAK] = find_column_of_terminal_in_table("break");
}

void free_rules() {
    Rule* cur;
    for (int i = 0; i < rules->size; i++) {
        cur = (Rule*)rules->array[i];
        //free(cur->nonterminal); <--- dont free nonterminal! used in nodes at syntax tree
        free(cur->ruleContent);
    }
    arraylist_free(rules);
}