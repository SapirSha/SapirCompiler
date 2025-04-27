#include "FileOut.h"
#include "ErrorHandler.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXIMUM_FILE_PATH 128

char* output_place;

char* extract_dir(const char* fullpath) {
    const char* last_bs = strrchr(fullpath, '\\');
    const char* last_sl = strrchr(fullpath, '/');
    const char* sep = last_bs > last_sl ? last_bs : last_sl;

    if (!sep) {
        return NULL;
    }

    unsigned int dir_len = sep - fullpath;
    char* dir = malloc(dir_len + 1);
    if (!dir) return NULL;

    memcpy(dir, fullpath, dir_len);
    dir[dir_len] = '\0';
    return dir;
}

void file_out(char* msg) {
    static bool is_first_call = true;
    char full_path[MAXIMUM_FILE_PATH];
    const char* filename = "output.asm";

    if (output_place && output_place[0] != '\0') {
        size_t dir_len = strlen(output_place);
        bool needs_separation = (output_place[dir_len - 1] != '/' && output_place[dir_len - 1] != '\\');
        snprintf(full_path, sizeof(full_path), "%s%s%s",
            output_place,
            needs_separation ? "/" : "",
            filename);
    }
    else {
        snprintf(full_path, sizeof(full_path), "%s", filename);
    }

    FILE* f = fopen(full_path, is_first_call ? "w" : "a");
    if (!f) {
        fprintf(stdout, "Unable to create Output file at %s\n", full_path);
        return;
    }

    fputs(msg, f);

    fclose(f);
    is_first_call = false;
}
