#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringIn.h"

int main() {
    const char* code = "\nint main() {\nint x = 42.57;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x >= 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    //const char* code = "int \"Hello;\"";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
}