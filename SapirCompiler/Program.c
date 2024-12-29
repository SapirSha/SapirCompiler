#include "Lexer.h"

int main() {
    const char* code = "c Hellow=7+World";
    printf("Tokenizing: %s\n", code);
    tokenize(code);
    return 0;
}