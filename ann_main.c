#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ann_matrix_ops.h"
#include "Thetas.h"

#define REMAP_PIXEL(p) {(((2.0*(p - 0))/255)-1)}

/* Configure ANN topology */
#define INPUT_LAYER_SIZE (64*60)
#define HIDDEN_LAYER_SIZE 25
#define NUM_LABELS 2
#define INPUT_EXAMPLES 42

/* For debug purposes */
#define DEBUG_ON 0

void sigmoid_matrix(float**, int, int, float**);
void add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);
int predict(float**, int, int, float**, int, int, float**, int, int);

int main (int argc, char *argv[])
{
   float **a, **c, ***pTheta2;
   char **cc;
   char *fileName = "kur";
   int csvRows, csvCols, i, j, status;
   
   if(argc < 2)
   {
       printf("Give me an input file.\n");
       exit(-1);
   }

   fileName = argv[1];

   //read_matrix_char(fileName, &cc, &csvRows, &csvCols);
//   read_matrix(fileName, &c, &csvRows, &csvCols);
	
//   printf("Printing content of the matrix as float (%dx%d).\n", csvRows, csvCols);
//   print_matrix(c, csvRows, csvCols);
/*
   printf("Print matrix as char (%dx%d).\n", csvRows, csvCols);
   print_matrix_char(cc, csvRows, csvCols);
*/
//   allocate_matrix_floats(&a, csvRows, csvCols);

//   printf("Scaling pixel data to [-1, 1] ... ");
//   for(i = 0; i < csvRows; i++)
//   {
//       for(j = 0; j < csvCols; j++)
//       {
//	   a[i][j] = (float)REMAP_PIXEL(c[i][j]);
//       }
//   }
   
//   printf("OK.\nPrint remapped/scaled matrix [-1;1] (%dx%d).\n", csvRows, csvCols);
//   print_matrix(a, csvRows, csvCols);
   

   float **Theta1Copy, **Theta2Copy, **testFaceCopy;

   allocate_matrix_floats(&Theta1Copy, HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1); 
   allocate_matrix_floats(&Theta2Copy, NUM_LABELS, HIDDEN_LAYER_SIZE+1);
   allocate_matrix_floats(&testFaceCopy, INPUT_EXAMPLES, INPUT_LAYER_SIZE);

   printf("Copy Theta1 ... ");
   for(i=0; i < HIDDEN_LAYER_SIZE; i++)
   {
       for(j=0; j < INPUT_LAYER_SIZE+1; j++)
       {
           Theta1Copy[i][j] =  Theta1[i][j];	   
       }
   }
   printf("Done.\n");

   printf("Copy Theta2 ... ");
   for(i=0; i < NUM_LABELS; i++)
   {
       for(j=0; j < HIDDEN_LAYER_SIZE+1; j++)
       {
           Theta2Copy[i][j] =  Theta2[i][j];	   
       }
   }
   printf("Done.\n");
   
   printf("Copy testFace ... ");
   for(i=0; i < INPUT_EXAMPLES; i++)
   {
       for(j=0; j < INPUT_LAYER_SIZE; j++)
       {
           testFaceCopy[i][j] =  testFace[i][j];	   
       }
   }
   printf("Done.\n");

   printf("BEGIN predict ... \n");
   status = predict(Theta1Copy, HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, Theta2Copy, NUM_LABELS, HIDDEN_LAYER_SIZE+1, testFaceCopy, INPUT_EXAMPLES, INPUT_LAYER_SIZE);

   printf("Predict status: %d ... ", status);
   if(status == 0)
   {     
       printf("OK.\n");
   }
   else
   {
      printf("Error.\n");
   }

   printf("Deallocate Theta1Copy ... ");
   deallocate_matrix_floats(&Theta1Copy, 25); 
   printf("Done.\n");
   printf("Deallocate Theta2Copy ... ");
   deallocate_matrix_floats(&Theta2Copy, 2);
   printf("Done.\n");
   printf("Deallocate testFaceCopy ... ");
   deallocate_matrix_floats(&testFaceCopy, 42);
   printf("Done.\n");

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

void ANN_config()
{
}


/** \fn int predict(float **mTheta1, float **mTheta2, float **mX)
    \brief Predict the label of an input given a trained neural network

     PREDICT(Theta1, Theta2, X) outputs the predicted label of X given the
     trained weights of a neural network (Theta1, Theta2)

     \param[in] mTheta1 a pointer to the 2D matrix of layer 1 weigth (Theta1) coeficients;
     \param[in] mTheta2 a pointer to the 2D matrix of layer 2 weigth (Theta2) coeficients;
     \param[in] mX      a pointer to the 2D matrix of input (pixel) values;
     \return the predicted label (1,2,3, etc.)
*/
int predict(float **mTheta1, int Theta1Rows, int Theta1Cols, float **mTheta2, int Theta2Rows, int Theta2Cols, float **mX, int XRows, int XCols)
{
   float **biasedMatrix, **tempMatrix;
   float **h1Matrix, **h2Matrix;
   int status = 1;

   /* allocate memoty for transposed Theta1, biased X and result h1 */
   allocate_matrix_floats(&tempMatrix, Theta1Cols, Theta1Rows); // holds transposed Theta1
   allocate_matrix_floats(&h1Matrix, XRows, Theta1Cols); // holds h1
   allocate_matrix_floats(&biasedMatrix, XRows, XCols+1);
   add_bias_column_to_matrix(mX, XRows, XCols, biasedMatrix);
   transpose_matrix(mTheta1, Theta1Rows, Theta1Cols, tempMatrix);
  
   if(((XCols+1) == Theta1Cols))
   {
       printf("X and Theta1' matrices OK - %dx%d * %dx%d.\n", XRows, XCols+1, Theta1Cols, Theta1Rows);
       status = ALG_MATMUL2D(XRows, Theta1Rows, Theta1Cols, biasedMatrix, tempMatrix, h1Matrix);
       sigmoid_matrix(h1Matrix, XRows, Theta1Rows, h1Matrix);
#if(DEBUG_ON == 1)
       printf("Sigmoid h1 result - (%dx%d).\n", XRows, Theta1Rows);
       print_matrix(h1Matrix, XRows, Theta1Rows);
#endif       
   }
   else
   {
       printf("X and Theta1 not compatible for multiplication: %dx%d * %dx%d.\n", XRows, XCols+1, Theta1Cols, Theta1Rows); 
       return -1;
   }

   deallocate_matrix_floats(&tempMatrix, XCols);
   deallocate_matrix_floats(&biasedMatrix, XRows);

   /****************************************/
   
   allocate_matrix_floats(&tempMatrix, Theta2Cols, Theta2Rows); // holds transposed Theta2
   allocate_matrix_floats(&h2Matrix, XRows, Theta2Rows); // holds h2
   allocate_matrix_floats(&biasedMatrix, XRows, Theta1Rows+1);
   add_bias_column_to_matrix(h1Matrix, XRows, Theta1Rows, biasedMatrix);
   transpose_matrix(mTheta2, Theta2Rows, Theta2Cols, tempMatrix);
  
   if((Theta1Rows+1) == Theta2Cols)
   {
       printf("h1 and Theta2' matrices OK - %dx%d * %dx%d.\n", XRows, Theta1Rows+1, Theta2Cols, Theta2Rows);
       status = ALG_MATMUL2D(XRows, Theta2Rows, Theta2Cols, biasedMatrix, tempMatrix, h2Matrix);
       sigmoid_matrix(h2Matrix, XRows, Theta2Rows, h2Matrix);
#if(DEBUG_ON == 1)
       printf("Sigmoid h2 result - (%dx%d).\n", XRows, Theta2Rows);
       print_matrix(h2Matrix, XRows, Theta2Rows);
#endif       
   }
  else
   {
       printf("h1 and Theta2 not compatible for multiplication: %dx%d * %dx%d.\n", XRows, Theta1Rows+1, Theta2Cols, Theta2Rows);
       return -1;
   }

   /* In h2Matrix are the results of prediction:
      at index 1 the posibility the image to be a face;
      at index 2 the posibility the image not to be a face;
      Get the index of max element in the row and tell if the image is face or not.
    */
   int i;
   int truePositives = 0, trueNegatives = 0;
   int falsePositives = 0, falseNegatives = 0;
   for(i=0; i < XRows; i++)
   {
      if(h2Matrix[i][0] > h2Matrix[i][1]) // A Face
      {
          printf("A Face at %d.", i);
          if(trueLabels[i][0] == 1)
          {
             printf("\tTrue.\n");
             truePositives++;
          }
          else
          {
             printf("\tFalse.\n");
             falsePositives++;
          }
      }
      else if(h2Matrix[i][0] == h2Matrix[i][1])
      {
          printf("50/50 at %d.\n", i);
      }
      else
      {
          printf("NOT a Face at %d.", i);
          if(trueLabels[i][0] == 1)
          {
             printf("\tFalse.\n");
             falseNegatives++;
          }
          else
          {
             printf("\tTrue.\n");
             trueNegatives++;
          }
      }      
    
   }

   printf("Deallocate all local matrices.\n");
   deallocate_matrix_floats(&tempMatrix, Theta2Cols);
   deallocate_matrix_floats(&biasedMatrix, XRows);
   deallocate_matrix_floats(&h1Matrix, XRows);
   deallocate_matrix_floats(&h2Matrix, XRows);
 
   printf("True Positives: %d.\nTrue Negatives: %d.\nFalse Positives: %d.\nFlase Negatives: %d.\n",\
          truePositives, trueNegatives, falsePositives, falseNegatives);


   return status;
}
