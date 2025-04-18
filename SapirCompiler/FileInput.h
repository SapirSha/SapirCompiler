#ifndef FILE_INPUT_H
#define FILE_INPUT_H
#include <stdbool.h>


char* get_possible_path_name(const char* line);
char* get_file_input(char* file_name);
bool looks_like_path(const char* s);

#endif