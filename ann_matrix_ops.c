#include <stdio.h>
#include <stdlib.h>
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
     printf("%5.3f ", matrix[i][j]);
    }
    putchar('\n');
   }
   putchar('\n');
}
