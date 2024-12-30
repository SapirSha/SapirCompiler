#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"


int main() {
    const char* code = "for (int i = 5; i < 7; i++){\n}";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
}