#include <stdio.h>
#include <stdlib.h>
#include "Thetas.h"


int main (int argc, char *argv[])
{
   
}

int gen_random(void)
{
   int i;
   int rows = 10;
   int cols = 6;
   FILE *pFile;
   float *a;

   a = (float *)malloc(rows*cols*sizeof(float));
   for(i = 0; i < rows*cols; i++)
   {
      a[i] = (float)(1.0/i);
   }
   pFile = fopen("kur", "w");
   fwrite(&rows, sizeof(int), 1, pFile);
   fwrite(&cols, sizeof(int), 1, pFile);
   fwrite(a, sizeof(float), (rows*cols), pFile);
   fclose(pFile);
   free(a);
   return 0;
}

int gen_from_header(char *fileName)
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
