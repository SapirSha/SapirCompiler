#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringIn.h"



int main() {
    
    const char* code = "ifawasd if";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
}