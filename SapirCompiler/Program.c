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

void printINT(int* e) {
    if (e != NULL) printf("%d", *e);
    else printf("0");
}

void printTOKEN(Token* token) {
    printf("T%d:L%s", token->type, token->lexeme);
}

int main() {
    const char* code =
        "function is_prime gets int number returns int # 1 if prime, else false # "
        "{ "
        "  if number < 0 return 0 "
        "  if number <= 3 return 1 "
        "  for int i = 3 while i < number/2 "
        "    if number % i == 0 return 0 "
        "  change i = i + 2 "
        "  return 1 "
        "} "
        "bool isprime = call is_prime with 7 "
        "print isprime ";


    printf("Tokenizing: %s\n", code);
    Queue* tokens = tokenize(code);
    queue_print(tokens, printTOKEN);
    
    
    printf("\n\n\n");
    create_parser_tables();
    printf("\n\n\n");


    commit_parser(tokens);

    return 0;

}