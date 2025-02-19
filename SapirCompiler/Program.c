#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringIn.h"
#include "ParserTableGenerator.h"
#include <stdio.h>

int main() {
    
    //const char* code = "\nint main() {\nint x = 42.57;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x >= 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    const char* code = "5+7*2+1";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    printf("\n\n\n");
    create_parser_tables();
    
    return 0;
    
}