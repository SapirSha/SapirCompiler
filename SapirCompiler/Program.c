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
    printf("T%d:L%s", token->type, token->lexeme);
}


void printSTR(char** str) {
    printf("%s ", *str);
}



int main() {
    const char* code =
        "function pow gets int x, int y returns int { "
        "    if y <= 0 return 1  "
        "    int temp = call pow(x, y/2) "
        "    if y % 2 == 0 return temp * temp "
        "    return temp * temp * x "
        "}  "
        "get int num1 get int num2 "
        "int result = call pow(num1, num2) "
        "print result  "
        "get num1 "
        "print num1  "
        ;

    printf("Tokenizing: %s\n", code);
    Queue* tokens = tokenize(code);
    queue_print(tokens, printTOKEN);
    
    
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");


    SyntaxTree* syntax_tree = commit_parser(tokens);
    free_parser_table();

    sementic_analysis(syntax_tree);

    BasicBlock* mainblock = mainCFG(syntax_tree);

    mainblock = computeLiveness(mainblock);

    generate_code(mainblock);


    return 0;

}