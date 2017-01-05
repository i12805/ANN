#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "ann_matrix_ops.h"

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

/** \fn void allocate_matrix(float ***subs, int rows, int cols)
    \brief Allocate memry for a rows x cols matrix of floats

    \param[out] ***subs pointer to the destination matrix's address.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \return void
*/

/* TODO check errors -  if malloc returms NULL, then return the status */
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

void allocate_matrix_floats(float ***matrix, int rows, int cols)
{
   int i;
   *matrix = (float **)malloc(rows*cols*sizeof(float));
   if(*matrix == NULL)
   {
      printf("ERROR allocating memory for rows of a matrix %dx%d.\n", rows, cols);
      exit(-1);
   }
   for(i=0; i < rows; i++)
   {
       (*matrix)[i] = (float*)malloc(cols*sizeof(float));
       if((*matrix)[i] == NULL)
       { 
            printf("ERROR allocating memory for cols of a matrix %dx%d.\n", rows, cols);
            exit(-1);
       }
   }
}

void deallocate_matrix_floats(float ***matrix, int rows)
{
   int i;
   for(i=0; i < rows; i++)
   {
       free((*matrix)[i]);
   }
   free(*matrix);
   return;
}

void init_matrix(float **matrix, int rows, int cols)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
        matrix[i][j] = 0.0;
    }
   }
}
/** \fn void read_image(char *fileName, int rows, int cols, float ***dest_matrix)
    \brief Read a file of floats and store the floats in a 2D matrix with one row.

     The function expects a sequence of floats. For compatibility reasons data is stored in a 2D matrix.

    \param[in] *fileName pointer to the file name with data.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[out] ***dest_matrix  pointer to the 2D destination matrix.
    \param[in] row_index index of the current data in the destination matrix
    \return nothing.
*/
void read_image(char *fileName, int rows, int cols, float ***dest_matrix, int row_index)
{
   char errorMsg[80];
   FILE *pFile;

   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      sprintf(errorMsg, "Can't open file: '%s'", fileName);
      printf("%s\n", errorMsg);
    }
    else
    {
       fread((*dest_matrix)[row_index], sizeof(float), (rows*cols), pFile);
       fclose(pFile);
    }
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
   
   //allocate_matrix(subs, *rows, *cols);
   allocate_matrix_floats(subs, *rows, *cols);
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
     printf("%.5f ", matrix[i][j]);
    }
    putchar('\n');
   }
   putchar('\n');
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
 
 #pragma omp parallel shared(A,B,C) private(I,J,K,temp)
 {
	#pragma omp parallel for schedule(static)
	//#pragma omp parallel for private(J,K,temp)
		 
   		for (I=0; I<M; I++)
   		{
      			for (J=0; J<N; J++)
      			{
         			//C[I][J]=0.;
         			temp = 0.0;
         			for (K=0; K<P; K++)
         			{	
            				//C[I][J]=(C[I][J])+((A[I][K])*(B[K][J]));
			            	temp += A[I][K] * B[K][J];
        			 }
      				C[I][J] = temp;
      			}
  	 	}	
 	
 }
 return 0;
}


