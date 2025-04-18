#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ErrorHandler.h"
#include "FileInput.h"

#define MAX_LINE 256   
#define INITIAL_CAP 1024   
#define MULTIPLIER_FACTOR 2   

static char* extend_buffer(char* buffer, int* cap, int add_len)
{
    int needed = *cap + add_len + 1;    
    int new_cap = *cap * MULTIPLIER_FACTOR;
    while (new_cap < needed) {
        new_cap *= MULTIPLIER_FACTOR;
    }
    char* tmp = realloc(buffer, new_cap);
    if (!tmp) handle_out_of_memory_error();
    *cap = new_cap;
    return tmp;
}

static int is_end_input(const char* line)
{
    int len = strlen(line);
    int i = 0;
    while (i < len && isspace((unsigned char)line[i])) i++;
    if (i < len && line[i] == '$') return 1;

    while (len > 0 && isspace((unsigned char)line[len - 1])) len--;
    if (len > 0 && line[len - 1] == '$') return 1;

    return 0;
}

char* get_console_input(void)
{
    fprintf(stdin, "Please enter code or file path: ");

    char cur_line[MAX_LINE];
    int buf_cap = INITIAL_CAP;
    int buf_len = 0;
    char* all_input = malloc(buf_cap);
    if (!all_input) {
        handle_out_of_memory_error();
        return NULL;
    }
    all_input[0] = '\0';

    bool first_line = true;
    while (fgets(cur_line, sizeof(cur_line), stdin)) {
        if (first_line) {
            first_line = false;
            bool possible_file = looks_like_path(cur_line);
            if (possible_file) {
                char* file_name = get_possible_path_name(cur_line);
                char* code = get_file_input(file_name);
                free(file_name);
                return code;
            }
        }

        
        cur_line[strcspn(cur_line, "\n")] = '\0';

        char* dollar = strchr(cur_line, '$');
        int is_end = dollar != NULL;

        int copy_len = is_end
            ? (int)(dollar - cur_line)
            : (int)strlen(cur_line);
        
        while (copy_len > 0 && isspace((unsigned char)cur_line[copy_len - 1])) {
            copy_len--;
        }
        
        if (buf_len + copy_len + 2 > buf_cap) {
            all_input = extend_buffer(all_input, &buf_cap, copy_len + 1);
        }
        
        memcpy(all_input + buf_len, cur_line, copy_len);
        buf_len += copy_len;
        
        if (!is_end) {
            all_input[buf_len++] = '\n';
        }
        all_input[buf_len] = '\0';

        if (is_end) break;
    }

    return all_input;
}
