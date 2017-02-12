#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "ann_file_ops.h"

#define WIDTH 320
#define HEIGHT 240

int main(int argc, char *argv[])
{
   char *fileName = argv[1];
   FILE *pFile;
   char var;
   unsigned i=0, ret=0;
   SDL_bool done = SDL_FALSE;
   int width=0, height=0, maxColorValue=0;
   char imageType[3];

   /* initialize window to display the image */
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
        printf("Error SDL init: %s.\n", SDL_GetError());
	return(-1);
   }
   SDL_Window *win = SDL_CreateWindow("Drink more tea!", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
   if (win == NULL)
   {
	printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return(-1);
   }    

   SDL_Surface *screen = SDL_GetWindowSurface(win);
   if(screen == NULL)
   {
        SDL_DestroyWindow(win);
        printf("Error GetWindowSurface: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
   }

   Uint32 rmask = 0x000000FFU;
   Uint32 gmask = 0x0000FF00U;
   Uint32 bmask = 0x00FF0000U;
   Uint32 amask = 0x00000000U;
   
   SDL_Surface *frame = NULL;

  char *pixels = (char *)malloc(WIDTH*HEIGHT*sizeof(char)); 
   if(pixels == NULL)
   {
      printf("Cannot allocate memory.\n");
      return(-1);
   }

   while(!done)
   {
	SDL_Event event;
        while (SDL_PollEvent(&event))
	{
	   switch (event.type)
	   {
               case SDL_KEYDOWN:
	          if (event.key.keysym.sym == SDLK_ESCAPE)
	          {
		      done = SDL_TRUE;
	          }
	          break;
	       case SDL_QUIT:
	           done = SDL_TRUE;
	           break;
	   }
 	}

   system("streamer -c /dev/video0 -f pgm -q -o images/test.pgm");
   SDL_Delay(50);
   pFile = fopen(fileName, "rb");
   if(pFile == NULL)
   {
      printf("Cannot open %s.\n", fileName);
      return(-1);
   }

   fscanf(pFile, "%s", imageType);
   fscanf(pFile, "%d", &width);
   fscanf(pFile, "%d", &height);
   fscanf(pFile, "%d", &maxColorValue);

   i = 0;
   while(!feof(pFile))
   {
      ret += fscanf(pFile, "%c", &var);
      pixels[i++] = var;
   }
   fclose(pFile);
   
   
   if(ret == 0) // nothing has bee read */
   {
       printf("Nothing has been read from file.\n");
       return(-1);
   }

        unsigned int u32Pixels[width*height];
        for(int i = 0; i < width * height; i++)
        {
             u32Pixels[i] = (unsigned)0x00<<24 |
                            (unsigned)pixels[i]<<16 |
                            (unsigned)pixels[i]<< 8 |
                            (unsigned)pixels[i];
        }

	int depth = 32;
	int pitch = 4*width;

       frame = SDL_CreateRGBSurfaceFrom((void*)u32Pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);

	if(frame == NULL)
	{
		  printf("Error creating the frame: %s.\n", IMG_GetError());
		  return(-1);
	} 
	 
	//SDL_BlitSurface(frame, NULL, screen, NULL);
        SDL_Rect target_rect = { .x=10, .y=10, .w=64, .h=60 };
        SDL_BlitScaled(frame, NULL, screen, &target_rect);
	SDL_UpdateWindowSurface(win);
   
   } /* End of while(!done) */

     
    free(pixels);
    SDL_FreeSurface(frame);
    SDL_FreeSurface(screen);
    SDL_DestroyWindow(win);
    SDL_Quit();
   return 0;
}
// gcc -std=c99 -g -Wall display_jpg.c ann_file_ops.c -o display_jpg -D_REENTRANT -I. -I/usr/include/SDL2 -lSDL2 -lSDL2_image
