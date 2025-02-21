#ifndef MATRIX_H
#define MATRIX_H

typedef struct Matrix {
	void*** content;
	unsigned int rows_amount;
	unsigned int columns_amount;
	unsigned int object_byte_size;
} Matrix;

Matrix* matrix_init(unsigned int rows_amount, unsigned int columns_amount, unsigned int object_byte_size);
void matrix_print(Matrix* matrix, void print_func(void*));
void matrix_insert(Matrix* matrix, unsigned int row, unsigned int col, void* value);
void matrix_free(Matrix** matrix);
void add_column(Matrix** matrix);
void add_row(Matrix** matrix);

#endif