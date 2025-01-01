#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringIn.h"

int main() {
	StringIn* Finder = stringin_init();
	stringin_insertString(Finder, "int");
	stringin_insertString(Finder, "string");
	stringin_insertString(Finder, "if");
	stringin_insertString(Finder, "else");

	printf("Searching for 'if': %d\n", stringin_searchString(Finder, "if"));
    char* str = "iF";
    bool flag = 0;
	for (; *str != '\0'; str++) {
        if (stringin_next_key(&Finder, str) == NOT_FOUND) {
            printf("NOT FOUND\n"); // <- Change to identifier
            flag = 1;
            break;
        }
    }
    if (!flag)
        if (stringin_next_key(&Finder, str) == FOUND)
            printf("FOUND!\n");
        else
            printf("NOT FOUND\n"); // <- Change to identifier


    /*
    const char* code = "\nint main() {\nint x = 42;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x > 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
    */
}