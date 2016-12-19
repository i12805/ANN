#include <stdio.h>
#include <stdlib.h>

int main()
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
   return 0;
}
