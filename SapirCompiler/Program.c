#include "Lexer.h"
#include "HashMap.h"
#include "Tokens.h"
#include "StringIn.h"



int main() {
	StringIn* Finder = stringin_init();
	stringin_insert_string(Finder, "int");
	stringin_insert_string(Finder, "interface");
	stringin_insert_string(Finder, "main");
	stringin_insert_string(Finder, "if");
	stringin_insert_string(Finder, "else");
	stringin_insert_string(Finder, "while");
	stringin_insert_string(Finder, "for");
	stringin_insert_string(Finder, "return");
	stringin_insert_string(Finder, "break");
	stringin_insert_string(Finder, "continue");
	stringin_insert_string(Finder, "continuew");
	stringin_insert_string(Finder, "continueaw");



	stringin_print(Finder);

	printf("%d\n", stringin_search_string(Finder, "while"));
	printf("%d\n", stringin_search_string(Finder, "interface"));
	printf("%d\n", stringin_search_string(Finder, "if"));





    /*
    const char* code = "\nint main() {\nint x = 42;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x > 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
    */
}