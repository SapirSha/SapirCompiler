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
#include "ErrorHandler.h"


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
    char* code =
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
        "function fibonachi gets int index2 returns int{ "
        "  if index2 == 0 return 0 "
        "  if index2 == 1 return 1 "
        "  return call fibonachi(index2-1) + call fibonachi(index2-2) "
        "} "
        "print \"Fibonachi Function: \"    "
        "print \"Input fibonachi index: \" "
        "get int index3 "
        "int fibo_res = call fibonachi(index3) "
        "print fibo_res "
        ""
        "function is_prime gets int number returns bool{ "
        "   if number <= 0 return false "
        "   if number <= 3 return true  "
        "   for int index1 = 3 while index1 < number/2 "
        "       if number % index1 == 0 return false  "
        "   change index1 = index1 + 2    "
        "   return true  "
        "} "
        "print \"Prime Function: \" "
        "print \"Enter Number to find if he is prime: \"    "
        "get int number "
        "bool is_number_prime = call is_prime(number) "
        "if is_number_prime == true "
        "   print \"Number Is Prime!\"    "
        "else "
        "   print \"Number Is Not Prime!\"  "
        "function is_even gets int num returns bool{ "
        "      bool state = true        "
        "      while num > 0 {          "
        "         state = state != true "
        "          num = num - 1        "
        "      }                        "
        "      return state             "
        "}                              "
        "print \"Even Function: \"      "
        "print \"enter a number to find if he's even: \" "
        "get int num "
        "bool is_even_num = call is_even(num)           "
        "if is_even_num == true print \"number is even\""
        "else print \"number is not even\"              "
        ;

    compile(code);
    
    return 0;

}