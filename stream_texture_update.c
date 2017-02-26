#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
#include "ann_file_ops.h"

#define WIDTH 320
#define HEIGHT 240

#define REC_BOX_WIDTH 64
#define REC_BOX_HEIGHT 60

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


    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (ren == NULL)
    {
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
 

     /* Get the windos Surface */
     SDL_Surface *screen = SDL_GetWindowSurface(win);
     if(screen == NULL)
     {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        printf("Error GetWindowSurface: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
     }

     /* Create new Rect obj to hold scaled image */
     SDL_Rect target_rect = { .x = 0, .y = 0, .w = (int)REC_BOX_WIDTH, .h = (int)REC_BOX_HEIGHT };
     //SDL_Surface *frame = SDL_GetWindowSurface(win);
     //SDL_BlitScaled(frame, NULL, screen, &target_rect);
     SDL_SetClipRect(screen, &target_rect);
     SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, screen);
     SDL_FreeSurface(screen);
     //SDL_FreeSurface(frame);
     if (tex == NULL)
     {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        printf("SDL_CreateTextureFromSurface Error: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
 
     SDL_Rect bounding_box = { .x=10, .y=10, .w=64, .h=60 };
     SDL_SetRenderDrawColor(ren, 250, 250, 0, SDL_ALPHA_OPAQUE); // opaque = 255
     
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

     /* Get image data */
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
        ret += fscanf(pFile, "%c", &var); // get pixels as char
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
          u32Pixels[i] = (unsigned)(
                         (unsigned)0x00<<24 |
                         (unsigned)pixels[i]<<16 |
                         (unsigned)pixels[i]<< 8 |
                         (unsigned)pixels[i]);
     }

     int depth = 32;
     int pitch = 4*width;


     bounding_box.y = 0;
     bounding_box.x = 0;
 

     SDL_UpdateTexture(tex, NULL, (void*)u32Pixels, pitch);
     
     SDL_RenderClear(ren);
     /* Draw the texture */
     SDL_RenderCopy(ren, tex, NULL, NULL);
     /* Draw the bounding box */
     while( ((bounding_box.y + REC_BOX_HEIGHT) <= (int)HEIGHT-1) )
     {
         if((bounding_box.x + REC_BOX_WIDTH) >= (int)WIDTH-1)
         {
             bounding_box.x = 0;
             bounding_box.y += 1;
         }
         else
         {
             bounding_box.x += 1;
         }
         printf("(%d, %d)\n", bounding_box.x, bounding_box.y);
     } 
     SDL_RenderDrawRect(ren, &bounding_box);
     /* Update the screen */
     SDL_RenderPresent(ren);

   
   } /* End of while(!done) */

   
    free(pixels);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
   return 0;
}
// gcc -std=c99 -g -Wall display_jpg.c ann_file_ops.c -o display_jpg -D_REENTRANT -I. -I/usr/include/SDL2 -lSDL2 -lSDL2_image
