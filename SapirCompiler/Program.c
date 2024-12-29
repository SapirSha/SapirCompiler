#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"


int main() {
    const char* code = "while (Hello == World + 27)";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
}