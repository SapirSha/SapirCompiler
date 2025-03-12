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
void printINT(int* e) {
    if (e != NULL) printf("%d", *e);
    else printf("0");
}



int main() {
    
    const char* code = "while(x >5){x++}";
    printf("Tokenizing: %s\n", code);
    TokensQueue* tokens = tokenize(code);
    tokens_print(tokens);
    
    
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");


    commit_parser(tokens);




    return 0;

}