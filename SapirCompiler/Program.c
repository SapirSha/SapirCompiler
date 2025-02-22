#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringTrie.h"
#include "ParserTableGenerator.h"
#include "LinkedList.h"
#include <stdio.h>
#include "Matrix.h"
#include "ArrayList.h"

void printINT(int* e) {
    if (e != NULL) printf("%d", *e);
    else printf("0");
}



int main() {
    
    //const char* code = "\nint main() {\nint x = 42.57;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x >= 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    const char* code = "5+7.1*2+1";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    printf("\n\n\n");
    create_parser_tables();




    return 0;

}