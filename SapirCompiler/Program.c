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


void printINT(int* e) {
    if (e != NULL) printf("%d", *e);
    else printf("0");
}

void printTOKEN(Token* token) {
    printf("T%d:L%s", token->type, token->lexeme);
}


void printSTR(char** str) {
    printf("%s ", *str);
}

int main() {
    //(call is_divisible_by_2 with num) == true
    const char* code =
        "function add gets float num1, float num2 returns float{ "
        "      return num1 + num2              "
        "}                                     "
        "int num1 = 8       int num2 = 16      "
        "int result = call add with num1, num2 "
        "num1 = 0           num2 = 0           "
        ;

    printf("Tokenizing: %s\n", code);
    Queue* tokens = tokenize(code);
    queue_print(tokens, printTOKEN);
    
    
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");


    SyntaxTree* syntax_tree = commit_parser(tokens);
    
    sementic_analysis(syntax_tree);

    mainCFG(syntax_tree);

    return 0;

}