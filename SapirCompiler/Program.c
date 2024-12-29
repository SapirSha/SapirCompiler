#include "Lexer.h"
#include "HashMap.h"


int main() {
    const char* code = "5;w23";
    printf("Tokenizing: %s\n", code);
    tokenize(code);
    return 0;
}