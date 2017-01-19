#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ann_config.h"
#include "ann_matrix_ops.h"
#include "ann_file_ops.h"

#ifdef USE_PARALLELLA
#include <e-hal.h>
#include "common.h"
#else
#include <omp.h>
#endif

#define REMAP_PIXEL(p) {(((2.0*(p - 0))/255)-1)}

/* Image size - fixed. The program expects to receive a 64x60 pxs image as an input parameter */
#define IMG_WIDTH 64
#define IMG_HEIGHT 60

/* Configure ANN topology */
#define INPUT_LAYER_SIZE (IMG_WIDTH * IMG_HEIGHT)
#define HIDDEN_LAYER_SIZE 25
#define NUM_LABELS 2
#define INPUT_EXAMPLES 42

/* For debug purposes */
#define DEBUG_ON 0
#define CHECK_LABELS 0

void sigmoid_matrix(float**, int, int, float**);
void add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);
int *predict(float**, int, int, float**, int, int, float**, int, int);
int recognise(int, char**);



int main (int argc, char *argv[])
{
   if(argc < 2)
   {
       printf("Give me at least one input file.\n");
       exit(-1);
   }
   
   printf("recognised: %d.\n", recognise(argc-1, argv));
   return 0;
}

int recognise(int input_examples, char *image_list[])
{
   float **image;
   char *fileName;
   int i, j, return_value = 1;
   pgm_image_t imageData;
   
   
   image = allocate_matrix_floats(input_examples, INPUT_LAYER_SIZE, 0);
   if(image == NULL)
   {
      printf("Error allocating image.\n");
      return(-1);
   }

   for(i = 0; i < input_examples; i++)
   {
      fileName = image_list[i+1];

      /* Read input image and store it in preallocated matrix */
      printf("Read file %s - %d of %d... ", fileName, i+1, input_examples);
      read_image_file(fileName, &imageData);
      printf("Done.\n");

//TODO check width, height, colorValue and adjust accordingly parameters, check dimentions!!!

#if(DEBUG_ON == 1)
      for(j=0; j < (imageData.width*imageData.height); j++)
      {
          printf("%f ", (imageData.paiPixels)[j]);
      }   
      printf("\n");
#endif

      /* Remap/scale pixes values in the range [-1, 1] */
      printf("Scaling pixel data to [-1, 1] ... ");
      for(j = 0; j < INPUT_LAYER_SIZE; j++)
      {
	   image[i][j] = (float)REMAP_PIXEL(imageData.paiPixels[j]);
      }
      printf("Done.\n");
#if(DEBUG_ON == 1)   
      print_matrix(image, input_examples, INPUT_LAYER_SIZE);
#endif   
      printf("Free image data pointer ...");
      free(imageData.paiPixels);
      printf("Done.\n");
   }

   float **Theta1Copy, **Theta2Copy;

   Theta1Copy = allocate_matrix_floats(HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, 0); 
   Theta2Copy = allocate_matrix_floats(NUM_LABELS, HIDDEN_LAYER_SIZE+1, 0);
   if((Theta1Copy == NULL) || (Theta2Copy == NULL))
   {
       printf("Error allocating Theta copies.\n");
       return(-1);
   }

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
   
   printf("BEGIN predict ... \n");
   
   int *result = predict(Theta1Copy, HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, Theta2Copy, NUM_LABELS, HIDDEN_LAYER_SIZE+1, image, input_examples, INPUT_LAYER_SIZE);
   
   printf("Predict status: ... ");
   if(result != NULL)
   {     
       printf("OK.\n");
       for(i = 0; i < input_examples; i++)
       {
           printf("Image %s is %d.\n", image_list[i+1], (result[i]));
       }
       return_value = 0;
   }
   else
   {
      printf("Error.\n");
      return_value = -1;
   }

   int dealloc_status = 0;
   printf("Deallocate Thetas, image matrix, result ... ");
   dealloc_status = deallocate_matrix_floats(Theta1Copy, HIDDEN_LAYER_SIZE); 
   dealloc_status = deallocate_matrix_floats(Theta2Copy, NUM_LABELS);
   dealloc_status = deallocate_matrix_floats(image, input_examples);
   free(result);
   printf("Done - %s.\n", (dealloc_status?"Error!":"OK."));

   return return_value;
}


/** \fn vvoid sigmoid_matrix(float **matrix, int rows, iment cols, float **result)
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


/** \fn int *predict(float **mTheta1, float **mTheta2, float **mX)
    \brief Predict the label of an input given a trained neural network

     PREDICT(Theta1, Theta2, X) outputs the predicted label of X given the
     trained weights of a neural network (Theta1, Theta2)

     \param[in] mTheta1 a pointer to the 2D matrix of layer 1 weigth (Theta1) coeficients;
     \param[in] mTheta2 a pointer to the 2D matrix of layer 2 weigth (Theta2) coeficients;
     \param[in] mX      a pointer to the 2D matrix of input (pixel) values;
     \return a vector with the predicted labels (1,2,3, etc.) or NULL on error.
*/

int *predict(float **mTheta1, int Theta1Rows, int Theta1Cols, float **mTheta2, int Theta2Rows, int Theta2Cols, float **mX, int XRows, int XCols)
{
   float **biasedMatrix, **tempMatrix;
   float **h1Matrix, **h2Matrix;
   int status = 1, dealloc_status = 0;

   /* allocate memoty for transposed Theta1, biased X and result h1 */
   tempMatrix = allocate_matrix_floats(Theta1Cols, Theta1Rows, 0); // holds transposed Theta1
   h1Matrix   = allocate_matrix_floats(XRows, Theta1Cols, 0); // holds h1
   biasedMatrix = allocate_matrix_floats(XRows, XCols+1, 0);

   if((tempMatrix == NULL) || (h1Matrix == NULL) || (biasedMatrix == NULL))
   {
       printf("Error allocating temp, h1, biased.\n");
       return(NULL);
   }

   add_bias_column_to_matrix(mX, XRows, XCols, biasedMatrix);
   int transpose_status = transpose_matrix(mTheta1, Theta1Rows, Theta1Cols, tempMatrix);
  
   if(((XCols+1) == Theta1Cols))
   {
#if(DEBUG_ON == 1)      
        printf("X and Theta1' matrices OK - %dx%d * %dx%d.\n", XRows, XCols+1, Theta1Cols, Theta1Rows);
#endif
       status = ALG_MATMUL2D(XRows, Theta1Rows, Theta1Cols, biasedMatrix, tempMatrix, h1Matrix);
       if(status != 0)
       {
            printf("\nERR from matrix multiplication h1.\n");
            return(NULL);
       }

       sigmoid_matrix(h1Matrix, XRows, Theta1Rows, h1Matrix);
#if(DEBUG_ON == 1)
       printf("Sigmoid h1 result - (%dx%d).\n", XRows, Theta1Rows);
       print_matrix(h1Matrix, XRows, Theta1Rows);
#endif       
   }
   else
   {
       printf("X and Theta1 not compatible for multiplication: %dx%d * %dx%d.\n", XRows, XCols+1, Theta1Cols, Theta1Rows); 
       return(NULL);
   }

   deallocate_matrix_floats(tempMatrix, XCols);
   deallocate_matrix_floats(biasedMatrix, XRows);

   /****************************************/
   
   tempMatrix = allocate_matrix_floats(Theta2Cols, Theta2Rows, 0); // holds transposed Theta2
   h2Matrix = allocate_matrix_floats(XRows, Theta2Rows, 0); // holds h2
   biasedMatrix = allocate_matrix_floats(XRows, Theta1Rows+1, 0);
   
   if((tempMatrix == NULL) || (h2Matrix == NULL) || (biasedMatrix == NULL))
   {
       printf("Error allocating temp, h2, biased.\n");
       return(NULL);
   }

   add_bias_column_to_matrix(h1Matrix, XRows, Theta1Rows, biasedMatrix);
   transpose_status = transpose_matrix(mTheta2, Theta2Rows, Theta2Cols, tempMatrix);
  
   if((Theta1Rows+1) == Theta2Cols)
   {
#if(DEBUG_ON == 1)
       printf("h1 and Theta2' matrices OK - %dx%d * %dx%d.\n", XRows, Theta1Rows+1, Theta2Cols, Theta2Rows);
#endif

       status = ALG_MATMUL2D(XRows, Theta2Rows, Theta2Cols, biasedMatrix, tempMatrix, h2Matrix);
       if(status != 0)
       {
            printf("\nERR from matrix multiplication h2.\n");
            return(NULL);
       }

// TODO get sigmoid matrix into the parallel execution     
       sigmoid_matrix(h2Matrix, XRows, Theta2Rows, h2Matrix);

#if(DEBUG_ON == 1)
       printf("Sigmoid h2 result - (%dx%d).\n", XRows, Theta2Rows);
       print_matrix(h2Matrix, XRows, Theta2Rows);
#endif       
   }
  else
   {
       printf("h1 and Theta2 not compatible for multiplication: %dx%d * %dx%d.\n", XRows, Theta1Rows+1, Theta2Cols, Theta2Rows);
       return(NULL);
   }

   /* In h2Matrix are the results of prediction:
      at index 1 the posibility the image to be a face;
      at index 2 the posibility the image not to be a face;
      Get the index of max element in the row and tell if the image is face or not.
    */
   int i;
#if(CHECK_LABELS == 1)
   int truePositives = 0, trueNegatives = 0;
   int falsePositives = 0, falseNegatives = 0;
#endif
   int *result = (int *)malloc(XRows*sizeof(int));
   if(result == NULL)
   {
       printf("Caanot allocate result vector.\n");
       return(NULL);
   }
   for(i=0; i < XRows; i++)
   {
      if(h2Matrix[i][0] > h2Matrix[i][1]) // A Face
      {
          result[i] = 1;
          printf("A Face at %d.", i);
#if(CHECK_LABELS == 1)
          if(trueLabels[i][0] == 1)
          {
             printf("\tTrue.");
             truePositives++;
          }
          else
          {
             printf("\tFalse.");
             falsePositives++;
          }
#endif
          printf("\n");
      }
      else if(h2Matrix[i][0] == h2Matrix[i][1])
      {
          printf("50/50 at %d.\n", i);
      }
      else
      {
          result[i] = 2;
          printf("NOT a Face at %d.", i);
#if(CHECK_LABELS == 1)
          if(trueLabels[i][0] == 1)
          {
             printf("\tFalse.");
             falseNegatives++;
          }
          else
          {
             printf("\tTrue.");
             trueNegatives++;
          }
#endif
          printf("\n");
      }  
    
   }

   printf("Deallocate all local matrices ... ");
   dealloc_status += deallocate_matrix_floats(tempMatrix, Theta2Cols);
   dealloc_status += deallocate_matrix_floats(biasedMatrix, XRows);
   dealloc_status += deallocate_matrix_floats(h1Matrix, XRows);
   dealloc_status += deallocate_matrix_floats(h2Matrix, XRows);
   printf("Done - %s.\n", (dealloc_status?"Error!":"OK."));

#if(CHECK_LABELS == 1) 
   printf("True Positives: %d.\nTrue Negatives: %d.\nFalse Positives: %d.\nFlase Negatives: %d.\n",\
          truePositives, trueNegatives, falsePositives, falseNegatives);
#endif

   return(result);
}

int sigmoid_gradient_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
{
   int i, j, dealloc_status;
   float **sigmoid_temp = allocate_matrix_floats(rows, cols, 0);
   if(sigmoid_temp == NULL)
   {
      return(-1);
   }
   else
   {
      sigmoid_matrix(src_matrix, rows, cols, sigmoid_temp);
      for(i=0; i < rows; i++)
      {
         for(j=0; j < cols; j++)
         {
             dest_matrix[i][j] = sigmoid_temp[i][j] * (1 - sigmoid_temp[i][j]);
         }
      }
   }
   dealloc_status = deallocate_matrix_floats(sigmoid_temp, rows);
   return(0);
}

void nnCostFunction(float **Theta1, float **Theta2, int input_layer_size, int hidden_layer_size, int num_labels, int input_examples, float **X, float **y, float lambda, float *J, float **gradient)
{
   return;
}
