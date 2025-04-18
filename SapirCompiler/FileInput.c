#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileInput.h"
#include "ErrorHandler.h"
#include <string.h>
#include <stdbool.h>
#include "FileOut.h"
#include <ctype.h>

#define MAX_PATH_LENGTH 128

#pragma warning(disable:4996)

char* get_possible_path_name(const char* line) {
    unsigned int start = 0, end = strlen(line);

    while (start < end && isspace((unsigned char)line[start])) {
        start++;
    }
    while (end > start && isspace((unsigned char)line[end - 1])) {
        end--;
    }
    unsigned int len = end - start;
    if (len == 0 || len > MAX_PATH_LENGTH) {
        return 0;
    }

    char path[MAX_PATH_LENGTH + 1];
    memcpy(path, line + start, len);
    path[len] = '\0';

    return strdup(path);
}
bool looks_like_path(const char* s) {
    if (!s || !*s) return false;
    if (strlen(s) < 3)

    if (s[0] == '/')
        return true;

    if (isalpha((unsigned char)s[0]) && s[1] == ':' &&
        (s[2] == '\\' || s[2] == '/'))
        return true;

    if (strncmp(s, "./", 2) == 0 || strncmp(s, "../", 3) == 0)
        return true;

    const char* dot = strrchr(s, '.');
    if (dot && dot != s)
        return true;

    return false;
}

char* get_file_input(char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        char msg[256];
        sprintf_s(msg, sizeof(msg),
            "Unable to open file \"%s\"\n", filename);
        handle_other_errors(msg);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        char msg[256];
        sprintf_s(msg, sizeof(msg),
            "Failed to seek in \"%s\"\n", filename);
        handle_other_errors(msg);
        return NULL;
    }

    long lsize = ftell(f);
    if (lsize < 0L) {
        fclose(f);
        char msg[256];
        sprintf_s(msg, sizeof(msg),
            "Failed to tell position in \"%s\"\n", filename);
        handle_other_errors(msg);
        return NULL;
    }
    unsigned int size = (unsigned int)lsize;
    rewind(f);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        handle_out_of_memory_error();
        return NULL;
    }

    unsigned int nread = fread(buffer, 1, size, f);
    if (nread != size || ferror(f)) {
        fclose(f);
        char msg[256];
        sprintf_s(msg, sizeof(msg),
            "Only read %zu of %zu bytes from \"%s\"\n",
            nread, size, filename);
        handle_other_errors(msg);
        free(buffer);
        return NULL;
    }
    buffer[size] = '\0';

    fclose(f);

    
    output_place = extract_dir(filename);
    return buffer;
}
