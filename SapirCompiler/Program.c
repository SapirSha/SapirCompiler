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
	stringin_insert_string(Finder, "Strin");
	stringin_insert_string(Finder, "String");








	stringin_print(Finder);
	/*
	printf("%d\n", stringin_search_string(Finder, "String"));
	printf("%d\n", stringin_search_string(Finder, "Strin"));
	printf("%d\n", stringin_search_string(Finder, "continue"));
	printf("%d\n", stringin_search_string(Finder, "if"));
	*/

	char str[] = "String";
	int index = 0;
	StringIn* pos = Finder;
	char* clearance = pos->to_clear;
	while (true) {
		int res = stringin_next(&pos, &clearance, str[index]);
		if (res == NOT_FOUND) {
			printf(" ---Not found--- \n");
			break;
		}
		else if (res == FOUND) {
			printf(" ---FOUND--- \n");
			break;
		}
		index++;
	}


	stringin_free(Finder);





    /*
    const char* code = "\nint main() {\nint x = 42;\nstring s = \"Hello, world!\"; # This is a comment#\nif (x > 10) { # Multi_line comment Example # x = x + 1;\n}\n}";
    printf("Tokenizing: %s\n", code);
    Tokens* tokens = tokenize(code);
    tokens_print(tokens);
    return 0;
    */
}