#include <stdio.h>
#include <stdlib.h>


int read_image_file(char*);
int toInt(char*);

int main (int argc, char *argv[])
{
   printf("Read %d items from file %s.\n", read_image_file(argv[1]), argv[1]);

   return 0;
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

int read_image_file(char *fileName)
{
   FILE *pFile;
   int i=0, j=0, ret=999, rows, cols;
   char var, str[4];
   char imgType[2];
   
   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      printf("Cannot open %s.\n", fileName);
      return -1;
   }

   fscanf(pFile, "%s", imgType);
   fscanf(pFile, "%d", &cols);
   fscanf(pFile, "%d", &rows);

   int pixels[(rows*cols)];
   
   while(!feof(pFile))
   {
      ret = fscanf(pFile, "%c", &var);
      if((var == '\n')||(var == ' '))
      {
          str[j] = '\0';
          pixels[i++] =  toInt(str);
          j = 0;
      }
      else
      {
          str[j++] = var;
      }
   }
   fclose(pFile);

   printf("Type: %s\n", imgType);
   printf("Cols: %d\n", cols);
   printf("Rows: %d\n", rows);

   for(i=0; i < 320; i++)
   {
       for(j = 0; j < 12; j++)
       {
           printf("%d ", pixels[i*12+j]);
       }
       printf("\n");
   }
   return ret;
}

int toInt(char a[])
{
  int c, sign, offset, n;
 
  if (a[0] == '-')
  {  // Handle negative integers
    sign = -1;
  }
 
  if (sign == -1)
  {  // Set starting position to convert
    offset = 1;
  }
  else
  {
    offset = 0;
  }
 
  n = 0;
 
  for (c = offset; a[c] != '\0'; c++)
  {
    n = n * 10 + a[c] - '0';
  }
 
  if (sign == -1)
  {
    n = -n;
  }
 
  return n;
}
