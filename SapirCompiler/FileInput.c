#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ErrorHandler.h"
#include <string.h>

#define MAX_PATH_LENGTH 128

#pragma warning(disable:4996)

char* try_get_file(const char* line) {
    size_t start = 0, end = strlen(line);

    while (start < end && isspace((unsigned char)line[start])) {
        start++;
    }
    while (end > start && isspace((unsigned char)line[end - 1])) {
        end--;
    }
    size_t len = end - start;
    if (len == 0 || len > MAX_PATH_LENGTH) {
        return 0;
    }

    char path[MAX_PATH_LENGTH + 1];
    memcpy(path, line + start, len);
    path[len] = '\0';

    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        return strdup(path);
    }
    return NULL;
}


char* get_file_input(const char* filename) {
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
    size_t size = (size_t)lsize;
    rewind(f);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        handle_out_of_memory_error();
        return NULL;
    }

    size_t nread = fread(buffer, 1, size, f);
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
    return buffer;
}
