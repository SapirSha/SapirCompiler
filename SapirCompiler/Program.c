#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringTrie.h"
#include "ParserTableGenerator.h"
#include "LinkedList.h"
#include <stdio.h>
#include "Matrix.h"
#include "ArrayList.h"
#include "HashSet.h"
#include "Parser.h"
#include "Queue.h"
#include "ParserTableSymbols.h"
#include "Sementic.h"
#include "SyntaxTree.h"
#include "IR_CFG.h"
#include "IR_Liveness.h"
#include "CodeGeneration.h"


void printINT(int* e) {
    if (e != NULL) printf("%d", *e);
    else printf("0");
}

void printTOKEN(Token* token) {
    printf("T%d:L%s(%d,%d)", token->type, token->lexeme, token->row, token->col);
}


void printSTR(char** str) {
    printf("%s ", *str);
}



int main() {
    const char* code =
        "\"wasdwasda@ wasdwasd @\"@";
    /*
    const char* code =
        "function pow gets int x, int y returns int { "
        "    if y <= 0 return 1  "
        "    int temp = call pow(x, y/2) "
        "    if y % 2 == 0 return temp * temp "
        "    return temp * temp * x "
        "}  "
        "print \"Power Function:\" "
        "print \"Number to power: \" "
        "get int num1 "
        "print \"Power by: \""
        "get int num2 "
        "int result = call pow(num1, num2) "
        "print \"Result:\" "
        "print result "
        ""
        "function fibonachi gets int index returns int{ "
        "  if index == 0 return 0 "
        "  if index == 1 return 1 "
        "  return call fibonachi(index-1) + call fibonachi(index-2) "
        "} "
        "print \"Fibonachi Function: \"    "
        "print \"Input fibonachi index: \" "
        "get int index "
        "int fibo_res = call fibonachi(index) "
        "print fibo_res "
        
        "function is_prime gets int number returns bool{ "
        "   if number <= 0 return false "
        "   if number <= 3 return true  "
        "   for int index = 3 while index < number/2 "
        "       if number % index == 0 return false  "
        "   change index = index + 2    "
        "   return true  "
        "} "

        "print \"Prime Function: \"    "
        "print \"Enter Number to find if he is prime: \"    "
        "get int number "
        "bool is_number_prime = call is_prime(number) "
        "if is_number_prime == true "
        "   print \"Number Is Prime!\"    "
        "else "
        "   print \"Number Is Not Prime!\"    "



        ;
        */
    printf("Tokenizing: %s\n", code);
    Queue* tokens = tokenize(code);
    queue_print(tokens, printTOKEN);
    
    /*
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");
     

    SyntaxTree* syntax_tree = commit_parser(tokens);
    free_parser_table();

    sementic_analysis(syntax_tree);

    BasicBlock* mainblock = mainCFG(syntax_tree);

    mainblock = computeLiveness(mainblock);

    generate_code(mainblock);

    */
    return 0;

}