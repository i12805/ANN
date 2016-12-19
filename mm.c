#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define REMAP_PIXEL(p) {(((2.0*(p - 0))/255)-1)}

int M = 1000;
int N = 2000;
int P = 500;

void allocate_matrix(float***, int, int);
void read_matrix(char*, float***, int*, int*);
void print_matrix(float**, int, int);

void allocate_matrix_char(char***, int, int);
void read_matrix_char(char*, char***, int*, int*);
void print_matrix_char(char**, int, int);

int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C);
void sigmoid_matrix(float**, int, int, float**);


int main (int argc, char *argv[])
{
   float **a, **b;
   char **c;
   char errorMsg[80];
   FILE *pFile;
   char *fileName = "kur";
   int csvRows, csvCols, i, j;

   fileName = argv[1];
//   dataBuffer = (char *)malloc(csvRows*csvCols*sizeof(float));
//   allocate_matrix(&a, csvRows, csvCols);

   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      sprintf(errorMsg, "Can't open file: '%s'", fileName);
      perror(errorMsg);
      exit(-1);
   }
   else
   {
	read_matrix_char(fileName, &c, &csvRows, &csvCols);
   }	
   fclose(pFile);

   printf("Print matrix as char.\n");
   print_matrix_char(c, csvRows, csvCols);

   allocate_matrix(&a, csvRows, csvCols);

   for(i = 0; i < csvRows; i++)
   {
       for(j = 0; j < csvCols; j++)
       {
	   a[i][j] = (float)REMAP_PIXEL(c[i][j]);
       }
   }

   printf("Print matrix as float.\n");
   print_matrix(a, csvRows, csvCols);
   
   allocate_matrix(&b, csvRows, csvCols);

#ifdef EXP_DEBUG
#pragma omp parallel for private(i,j)
   for(i = 0; i < csvRows; i++)
   {
     for(j = 0; j < csvCols; j++)
     {
       b[i][j] = exp(a[i][j]);
     }
   }

   printf("Print exp matrix as float.\n");
   print_matrix(b, csvRows, csvCols);
#endif

   printf("Sigmoid on matrix.\n");
   sigmoid_matrix(a, csvRows, csvCols, b);
   print_matrix(b, csvRows, csvCols);
   
   return 0;
}



/* Matrix manipulation functions */

/** \fn void allocate_matrix_char(char ***subs, int rows, int cols)
    \brief Allocate memry for a rows x cols matrix of chars

    \param[in] ***subs pointer to the destination matrix's address.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \return void
*/

/* TODO check errors -  if malloc returms NULL, then return the status */

void allocate_matrix_char(char ***subs, int rows, int cols)
{
   int i;
   char *storage;

   storage = (char *)malloc(rows*cols*sizeof(char));
   *subs = (char **)malloc(rows*sizeof(char *));

   for(i = 0; i < rows; i++)
   {
     (*subs)[i] = &storage[i*cols];
   }
   return;
}

void read_matrix_char(char *fileName, char ***subs, int *rows, int *cols)
{
   char errorMsg[80];
   FILE *pFile;

   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      sprintf(errorMsg, "Can't open file: '%s'", fileName);
      printf("%s\n", errorMsg);
   }
   //fread(rows, sizeof(int), 1, pFile);
   //fread(cols, sizeof(int), 1, pFile);
   
   *rows = 64;
   *cols = 60;

   allocate_matrix_char(subs, *rows, *cols);
   fread((*subs)[0], sizeof(char), ((*rows) * (*cols)), pFile);
   fclose(pFile);
}

void print_matrix_char(char **matrix, int rows, int cols)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     printf("%d ",matrix[i][j]);
    }
    printf("\n");
   }
   putchar('\n');
}


/* Matrix aux functions for floats */
void allocate_matrix(float ***subs, int rows, int cols)
{
   int i;
   float *storage;

   storage = (float *)malloc(rows*cols*sizeof(float));
   *subs = (float **)malloc(rows*sizeof(float *));

   for(i = 0; i < rows; i++)
   {
     (*subs)[i] = &storage[i*cols];
   }
   return;
}

void read_matrix(char *fileName, float ***subs, int *rows, int *cols)
{
   char errorMsg[80];
   FILE *pFile;

   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      sprintf(errorMsg, "Can't open file: '%s'", fileName);
      printf("%s\n", errorMsg);
   }
   fread(rows, sizeof(int), 1, pFile);
   fread(cols, sizeof(int), 1, pFile);
   
   allocate_matrix(subs, *rows, *cols);
   fread((*subs)[0], sizeof(float), ((*rows) * (*cols)), pFile);
   fclose(pFile);
}

void print_matrix(float **matrix, int rows, int cols)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     printf("%2.4f ",matrix[i][j]);
    }
    printf("\n");
   }
   putchar('\n');
}

/****************** Matrix Calculations ***************************/

/** \fn int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C)
    \brief Multiplies two 2D matrices of floats.

    \param[in] M number of rows of the first multiplier matrix.
    \param[in] N number of columns of the second multiplier matrix.        
    \param[in] P number of columns of the first multiplier, resp. rows of the first multipl.  matrix.
    \param[in] **A pointer to the first miltiplier - a 2D matrix of floats.
    \param[in] **B pointer to the second multiplier - a 2D matrix of floats.
    \param[out] **C pointer to the result (2D) matrix of floats.
    \return zero on success.
*/

int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C)
{
   int I=0, J=0, K=0;
   float temp = 0.0;
   printf("start mmult\n");
//#pragma omp parallel shared(A,B,C) private(I,J,K,temp) //PRAGMA OMP PARALLEL SHARED(A,B,C) PRIVATE(I,J,K)
  {
//#pragma omp parallel for schedule(static) //SCHEDULE(STATIC)
//#pragma omp parallel for private(J,K,temp)
   for (I=0; I<M; I=I+1)
   {
      for (J=0; J<N; J=J+1)
      {
         //C[I][J]=0.;
         temp = 0.0;
         for (K=0; K<P; K=K+1)
         {
            //C[I][J]=(C[I][J])+((A[I][K])*(B[K][J]));
            temp += A[I][I] * B[K][J];
         }
       C[I][J] = temp;
      }
   }
  }
   return 0;
}

float remap_pixel(char p)
{
   return (((2.0*(float)(p))/255.0)-1.0);
}


/** \fn vvoid sigmoid_matrix(float **matrix, int rows, int cols, float **result)
    \brief Calculates the sigmoid function over the given 2D matrix of floats.

    \param[in] **matrix pointer to the 2D source matrix.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[out] **result (matrix) pointer to the 2D destination(i.e. result) matrix.
    \return nothing.
*/

void sigmoid_matrix(float **matrix, int rows, int cols, float **result)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     result[i][j] = (1.0 / (1.0 + exp(-matrix[i][j])));
     
    }
   }
}

/** \fn void transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
    \brief Transpose a 2D matrix.

    \param[in] **src_matrix  pointer to the 2D source matrix.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[out] **dest_matrix  pointer to the 2D destination(i.e. transposed) matrix.
    \return nothing.
*/

void transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
{
   
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     dest_matrix[j][i] = src_matrix[i][j];
    }
   }
}

int predict(float **mTheta1, float **mTheta2, float **mX)
{
   return 0;
}
