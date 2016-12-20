#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ann_matrix_ops.h"

#define REMAP_PIXEL(p) {(((2.0*(p - 0))/255)-1)}

int M = 1000;
int N = 2000;
int P = 500;


void sigmoid_matrix(float**, int, int, float**);
void add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);

int main (int argc, char *argv[])
{
   float **a, **b, **c, **biasedMatrix;
   char **cc;
   char errorMsg[80];
   FILE *pFile;
   char *fileName = "kur";
   int csvRows, csvCols, i, j, status;

   if(argc < 2)
   {
       printf("Give me an input file.\n");
       exit(-1);
   }

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
	//read_matrix_char(fileName, &cc, &csvRows, &csvCols);
        read_matrix(fileName, &c, &csvRows, &csvCols);
   }	
   fclose(pFile);

   printf("Print matrix as float (%dx%d).\n", csvRows, csvCols);
   print_matrix(c, csvRows, csvCols);
/*
   printf("Print matrix as char (%dx%d).\n", csvRows, csvCols);
   print_matrix_char(cc, csvRows, csvCols);
*/
   allocate_matrix(&a, csvRows, csvCols);

   for(i = 0; i < csvRows; i++)
   {
       for(j = 0; j < csvCols; j++)
       {
	   a[i][j] = (float)REMAP_PIXEL(c[i][j]);
       }
   }
   
   printf("Print remapped matrix [-1;1] (%dx%d).\n", csvRows, csvCols);
   print_matrix(a, csvRows, csvCols);
   
   allocate_matrix(&b, csvRows, csvCols);

   printf("Sigmoid on matrix A.\n");
   sigmoid_matrix(a, csvRows, csvCols, b);
   print_matrix(b, csvRows, csvCols);
  
   printf("Transposed matrix A.\n");
   transpose_matrix(a, csvRows, csvCols, b);   
   print_matrix(b, csvCols, csvRows);
      
   printf("Biased matrix A.\n");
   allocate_matrix(&biasedMatrix, csvRows, csvCols+1);
   add_bias_column_to_matrix(a, csvRows, csvCols, biasedMatrix);   
   print_matrix(biasedMatrix, csvRows, csvCols+1);

   
 //  deallocate_matrix(&a, csvRows, csvCols);
 //  deallocate_matrix(&b, csvRows, csvCols);
 //  deallocate_matrix(&biasedMatrix, csvRows, csvCols+1);
   return 0;
}




/****************** Matrix Calculations ***************************/


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


/** \fn int add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
    \brief Adds a column with ones at position 0.

    Adds a bias vector as the first column to a matrix. Matrix should be allocated first.

    \param[in] **src_matrix  pointer to the 2D source matrix.
    \param[in] rows number of rows of the sorce matrix.
    \param[in] cols number of columns of the source matrix.
    \param[out] **dest_matrix  pointer to the 2D destination(i.e. with bias vector added) matrix.
    \return zero on success.
*/

void add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    dest_matrix[i][0] = 1.0;
    for(j=0; j < cols; j++)
    {
        dest_matrix[i][j+1] = src_matrix[i][j];
     }
   }
}

/** \fn int predict(float **mTheta1, float **mTheta2, float **mX)
    \brief Predict the label of an input given a trained neural network

     PREDICT(Theta1, Theta2, X) outputs the predicted label of X given the
     trained weights of a neural network (Theta1, Theta2)

     \param[in] mTheta1 a pointer to the 2D matrix of Theta1 coeficients;
     \param[in] mTheta2 a pointer to the 2D matrix of Theta2 coeficients;
     \param[in] mX      a pointer to the 2D matrix of input coeficients;
     \return the predicted label (1,2,3, etc.)
*/
int predict(float **mTheta1, int Theta1Rows, int Theta1Cols, float **mTheta2, int Theta2Rows, int Theta2Cols, float **mX, int XRows, int XCols)
{
   float **biasedMatrix, **tempMatrix;
   float **resultMatrix;
   int status = 1;

   allocate_matrix(&tempMatrix, XCols, XRows);
   allocate_matrix(&resultMatrix, XRows, Theta1Cols);
   allocate_matrix(&biasedMatrix, XRows, XCols+1);
   add_bias_column_to_matrix(mX, XRows, XCols, biasedMatrix);
   transpose_matrix(mTheta1, Theta1Rows, Theta1Cols, tempMatrix);
   
   if(((XCols+1) == Theta1Cols))
   {
       printf("X and Theta1' matrices OK - %dx%d * %dx%d.\n", XRows, XCols+1, Theta1Cols, Theta1Rows);
       status = ALG_MATMUL2D(XRows, Theta1Rows, Theta1Cols, biasedMatrix, mTheta1, resultMatrix);
       print_matrix(resultMatrix, XRows, Theta1Rows);
   }
   else
   {
       printf("X and Theta1 not compatible for multiplication.\n"); 
   }

   deallocate_matrix(&tempMatrix, XCols, XRows);
   deallocate_matrix(&resultMatrix, XRows, Theta1Cols);
   deallocate_matrix(&biasedMatrix, XRows, XCols+1);
 
  return status;
}
