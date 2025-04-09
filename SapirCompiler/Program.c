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
		"function power gets int x, int y returns int{ "
        "   break "
        "} "
        "int result = call power(7, 10) "
        ;

    printf("Tokenizing: %s\n", code);
    Queue* tokens = tokenize(code);
    queue_print(tokens, printTOKEN);
    
    
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");


    SyntaxTree* syntax_tree = commit_parser(tokens);
    
    sementic_analysis(syntax_tree);

    BasicBlock* mainblock = mainCFG(syntax_tree);

    mainblock = computeLiveness(mainblock);



    return 0;

}