#include <stdio.h>
#include <stdlib.h>
#include "ann_file_ops.h"



int toInt(char*);


int read_image_file(char *fileName, pgm_image_t *image)
{
   FILE *pFile;
   int i=0, j=0, ret=999, rows, cols, maxColorValue;
   char var, str[4];
   char imgType[3];
   
   pFile = fopen(fileName, "r");
   if(pFile == NULL)
   {
      printf("Cannot open %s.\n", fileName);
      return -1;
   }

   fscanf(pFile, "%s", imgType);
   fscanf(pFile, "%d", &cols);
   fscanf(pFile, "%d", &rows);
   fscanf(pFile, "%d", &maxColorValue);

   float *pixels = (float *)malloc(rows*cols*sizeof(float));
   
   while(!feof(pFile))
   {
      ret = fscanf(pFile, "%c", &var);
      if((var == '\n')||(var == ' '))
      {
          if(j == 0) continue;
          str[j] = '\0';
          pixels[i++] = (float)toInt(str);
          j = 0;
      }
      else
      {
          str[j++] = var;
      }
   }
   fclose(pFile);
#ifdef DEBUG_ON
   for(i=0; i < 320; i++)
   {
       for(j=0; j < 12; j++)
       {
           printf("%f ", pixels[i*12+j]);
       }
       printf("\n");
    }    
#endif
   image->imageType[0] = imgType[0];
   image->imageType[1] = imgType[1];
   image->imageType[2] = imgType[2];
   image->width = cols;
   image->height = rows;
   image->maxColorValue = maxColorValue;
   image->paiPixels = pixels;
#ifdef DEBUG_ON
   printf("image type: %s\n", image->imageType);
   printf("image width: %d\n", image->width);
   printf("image height: %d\n", image->height);
   printf("image color value: %d\n", image->maxColorValue);
   for(i=0; i < (rows*cols); i++)
   {
       printf("%f ", image->paiPixels[i]);
   }
   printf("\n\n");
#endif
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
