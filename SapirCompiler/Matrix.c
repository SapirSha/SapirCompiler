#include "Matrix.h"
#include <stdlib.h>
#include <string.h>

void matrix_free(Matrix** matrix) {
    Matrix* m = *matrix;
    for (int row = 0; row < m->rows_amount; row++) {
        for (int col = 0; col < m->columns_amount; col++) {
            if (m->content[row][col] != NULL) free(m->content[row][col]);
        }
    }
    free(*m->content);
    free(m->content);
    free(m);
    *matrix = NULL;
}

Matrix* matrix_init(unsigned int rows_amount, unsigned int columns_amount, unsigned int object_byte_size) {
    Matrix* matrix = malloc(sizeof(Matrix));
    matrix->columns_amount = columns_amount;
    matrix->rows_amount = rows_amount;
    matrix->object_byte_size = object_byte_size;

    void*** rows_pointers = malloc(rows_amount * sizeof(void**));

    void** content = calloc(rows_amount * columns_amount, object_byte_size);

    for (unsigned int i = 0; i < rows_amount; i++) {
        rows_pointers[i] = content + i * columns_amount;
    }

    matrix->content = rows_pointers;
    return matrix;
}



void matrix_print(Matrix* m, void print_func(void*)) {
    for (unsigned int row = 0; row < m->rows_amount; row++) {
        for (unsigned int col = 0; col < m->columns_amount; col++) {
            print_func(m->content[row][col]);
            printf(", ");
        }
        printf("\n");
    }
}

void matrix_insert(Matrix* matrix, unsigned int row, unsigned int col, void* value) {
    if (matrix->content[row][col] != NULL) free(matrix->content[row][col]);
    matrix->content[row][col] = malloc(matrix->object_byte_size);
    memcpy(matrix->content[row][col], value, matrix->object_byte_size);
}

void add_column(Matrix** matrix) {
    Matrix* mat = *matrix;
    // Create a new matrix with an extra column.
    Matrix* new_mat = matrix_init(mat->rows_amount, mat->columns_amount + 1, mat->object_byte_size);
    if (!new_mat) {
        perror("Failed to initialize new matrix in add_column");
        return;
    }
    // Copy data from the old matrix to the new one.
    for (unsigned int row = 0; row < mat->rows_amount; row++) {
        for (unsigned int col = 0; col < mat->columns_amount; col++) {
            if (mat->content[row][col] != NULL) {
                // Insert into new_mat (note: use new_mat here, not mat)
                matrix_insert(new_mat, row, col, mat->content[row][col]);
            }
        }
    }
    // Free the old matrix (this function should free both the contiguous block and the row pointers)
    matrix_free(*matrix);
    *matrix = new_mat;
}


void add_row(Matrix** matrix) {
    Matrix* mat = *matrix;
    unsigned int new_rows = mat->rows_amount + 1;

    // Retrieve the current contiguous block.
    // (Assuming that if rows_amount > 0, then mat->content[0] is the block.)
    void* old_block = (mat->rows_amount > 0) ? (void*)mat->content[0] : NULL;
    void* new_block;

    // Expand the contiguous block to hold new_rows * columns_amount cells.
    if (old_block) {
        new_block = realloc(old_block, new_rows * mat->columns_amount * mat->object_byte_size);
    }
    else {
        // If there was no block (matrix was empty), allocate new memory.
        new_block = calloc(new_rows * mat->columns_amount, mat->object_byte_size);
    }

    if (!new_block) {
        perror("Realloc failed for contiguous block in add_row");
        return;
    }

    // Reallocate the row pointers array so that it can hold new_rows pointers.
    void*** new_row_ptrs = realloc(mat->content, new_rows * sizeof(void**));
    if (!new_row_ptrs) {
        perror("Realloc failed for row pointers array in add_row");
        return;
    }
    mat->content = new_row_ptrs;

    // Update all row pointers so they point into the new contiguous block.
    for (unsigned int i = 0; i < new_rows; i++) {
        mat->content[i] = (void**)((char*)new_block + i * mat->columns_amount * mat->object_byte_size);
    }

    // Initialize the new row's cells to NULL.
    for (unsigned int col = 0; col < mat->columns_amount; col++) {
        mat->content[new_rows - 1][col] = NULL;
    }

    // Update the rows count.
    mat->rows_amount = new_rows;
}


