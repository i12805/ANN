#ifndef ANN_MATRIX_OPS_H
#define ANN_MATRIX_OPS_H 

void allocate_matrix(float***, int, int);
void allocate_matrix_floats(float***, int, int);
void deallocate_matrix_floats(float***, int);
void init_matrix(float**, int, int);
void read_matrix(char*, float***, int*, int*);
void read_image(char*, int, int, float***, int);
void print_matrix(float**, int, int);

void allocate_matrix_char(char***, int, int);
void read_matrix_char(char*, char***, int*, int*);
void print_matrix_char(char**, int, int);

void transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);
int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C);

#endif
