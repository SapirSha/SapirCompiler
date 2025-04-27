#include "Compiler.h"
#include "ConsoleInput.h"
#include "FileInput.h"
#include "ErrorHandler.h"
#include "ParserTableGenerator.h"
#include "FileOut.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    create_parser_tables();

    char* code = NULL;
    output_place = NULL;

    if (argc == 2)
        code = get_file_input(argv[1]);
    else 
        code = get_console_input();

    if (current_error_state != NO_ERROR) return 1;

    printf("\n\n\n\n\n");

    bool compiler_result = compile(code);

    free(code);
    if (compiler_result) {
        printf("\n\n\t ---Compiled successfully\n");
        return 0;
    }
    else {
        printf("\n\n\t ---Failed to compile code\n");
        return 1;
    }
}