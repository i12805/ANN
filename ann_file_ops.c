#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "ann_file_ops.h"



int toInt(char*);

/* read pgm image file in binarry mode */
pgm_image_t read_pgm_binary(char *fileName)
{
   FILE *pFile;
   unsigned int i=0;
   unsigned int ret=0;
   unsigned int rows, cols, maxColorValue;
   char var;
   char imgType[3];
   /* designated initializers */
   pgm_image_t image = {.imageType = "KR", .width = 0, .height = 0, .maxColorValue = 0, .paiPixels = NULL};
   
   pFile = fopen(fileName, "rb");
   if(pFile == NULL)
   {
      printf("Cannot open %s.\n", fileName);
      return(image);
   }

   fscanf(pFile, "%s", imgType);
   fscanf(pFile, "%d", &cols);
   fscanf(pFile, "%d", &rows);
   fscanf(pFile, "%d", &maxColorValue);

   float *pixels = (float *)malloc(rows*cols*sizeof(float)); 
   if(pixels == NULL)
   {
      printf("Cannot allocate memory.\n");
      return(image);
   }

   while(!feof(pFile))
   {
      ret += fscanf(pFile, "%c", &var);
      pixels[i++] = (float)var;
   }
   fclose(pFile);
   
   if(ret == 0) // nothing has bee read */
   {
       printf("Nothing has been read from file.\n");
       return(image);
   }

   /* populate the struct */
   image.imageType[0] = imgType[0];
   image.imageType[1] = imgType[1];
   image.imageType[2] = imgType[2];
   image.width = cols;
   image.height = rows;
   image.maxColorValue = maxColorValue;
   image.paiPixels = pixels;

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

   return image;
}


/** \fn int read_image_file(char *fileName, pgm_image_t *image)
    \brief read image file coded as zero quality pgm, i.e. pixels represented as ascii values
*/

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


int display_image_file(char *path_to_image, int width, int height)
{

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error SDL init: %s.\n", SDL_GetError());
	return -1;
    }

    if(IMG_Init(IMG_INIT_JPG) < 0)
    {
        printf("Error IMG init: %s.\n", IMG_GetError());
        return(-1);
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, width, height, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
	printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return -1;
    }    

    SDL_Surface *screen = SDL_GetWindowSurface(win);
    if(screen == NULL)
    {
        SDL_DestroyWindow(win);
        printf("Error GetWindowSurface: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
    }
    
    SDL_RWops *buffer_stream;
    SDL_Surface *frame;
    
    buffer_stream = SDL_RWFromFile(path_to_image, "rb");
    frame = IMG_LoadJPG_RW(buffer_stream);

    if(frame == NULL)
    {
          printf("Error loading the frame: %s.\n", IMG_GetError());
          return(-1);
    }

    SDL_PixelFormat *format = frame->format;
    if(format == NULL)
    {
          printf("Error getting pixel format: %s.\n", IMG_GetError());
          return(-1);
    }

    int total = (frame->h * frame->pitch);
    printf("Display frame:\n width: %d, height: %d pxls.\n pitch: %d bytes\n total pxls: %d bytes\n", frame->w, frame->h, frame->pitch, total);


  
    printf("\nformat:\n bits per pixel %d\n bytes per pixel %d\n RGBA masks 0x%08X 0x%08X 0x%08X 0x%08X\n", format->BitsPerPixel, format->BytesPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);

 //   printf("0x%08X ", ((int*)(frame->pixels))[i]);
 //   unsigned flags = 0x00000000;
  //  SDL_Surface converted_frame = SDL_ConvertSurface(frame, format, flags);

    
    SDL_BlitSurface(frame, NULL, screen, NULL);
    SDL_UpdateWindowSurface(win);
    SDL_Delay(1000);

    IMG_Quit();
    SDL_FreeSurface(frame);
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}


/** \brief Display image given the pixels in a memory buffer */

int display_image_mem(void *pixels, int width, int height)
{

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error SDL init: %s.\n", SDL_GetError());
	return -1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, width, height, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
	printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return -1;
    }    

    SDL_Surface *screen = SDL_GetWindowSurface(win);
    if(screen == NULL)
    {
        SDL_DestroyWindow(win);
        printf("Error GetWindowSurface: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
    }
    
    int depth = 32;
    int pitch = 4*width;
    Uint32 rmask = 0x000000FFU;
    Uint32 gmask = 0x0000FF00U;
    Uint32 bmask = 0x00FF0000U;
    Uint32 amask = 0x00000000U;

    SDL_Surface *frame = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);

    if(frame == NULL)
    {
          printf("Error creating the frame: %s.\n", IMG_GetError());
          return(-1);
    }

   
 
    SDL_BlitSurface(frame, NULL, screen, NULL);
    SDL_UpdateWindowSurface(win);
    SDL_Delay(3000);

    SDL_FreeSurface(frame);
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
