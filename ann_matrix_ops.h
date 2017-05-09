#ifndef ANN_MATRIX_OPS_H
#define ANN_MATRIX_OPS_H 

float **allocate_matrix_floats(int, int, float);
int  deallocate_matrix_floats(float**, int);
int init_matrix(float**, int, int, float);
void print_matrix(float**, int, int);

void allocate_matrix_char(char***, int, int);
void read_matrix_char(char*, char***, int*, int*);
void print_matrix_char(char**, int, int);

int transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);
int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C);

#endif /* ANN_MATRIX_OPS_H */
