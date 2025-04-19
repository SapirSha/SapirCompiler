#include "Compiler.h"
#include "ConsoleInput.h"
#include "FileInput.h"
#include "ErrorHandler.h"
#include "ParserTableGenerator.h"
#include "FileOut.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    char* code = NULL;
    output_place = NULL;

    if (argc == 2)
        code = get_file_input(argv[1]);
    else 
        code = get_console_input();

    if (current_error_state != NO_ERROR) return 0;

    printf("\n\n\n\n\n");

    bool compiler_result = compile(code);

    if (compiler_result)
        printf("\n\n\t ---Compiled successfully\n");
    else 
        printf("\n\n\t ---Failed to compile code\n");

    free(code);
    return 0;

}