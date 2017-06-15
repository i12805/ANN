#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#ifndef TEST
#include "ann_config.h"
#endif
#include "ann_matrix_ops.h"
#include "ann_file_ops.h"

#ifdef USE_PARALLELLA
#include <e-hal.h>
#else
#include <omp.h>
#endif

#define REMAP_PIXEL(p) {(((2.0*(p - 0))/255)-1)}

/* Image size - fixed. The program expects to receive a 64x60 pxs image as an input parameter */
#define IMG_WIDTH 64
#define IMG_HEIGHT 60

#define DISPLAY_WIN_WIDTH 320
#define DISPLAY_WIN_HEIGHT 240

#define FACES_FOUND_MAX 500

/* Configure ANN topology */
#define INPUT_LAYER_SIZE (IMG_WIDTH * IMG_HEIGHT)
#define HIDDEN_LAYER_1_SIZE 25
#define HIDDEN_LAYER_2_SIZE 25
#define NUM_LABELS 2
#define INPUT_EXAMPLES 42

/* For debug purposes */
#define DEBUG_ON 0 
#define CHECK_LABELS 0

#ifdef TEST
const float Theta1[1][1];
const float Theta2[1][1];
#endif
#define HIDDEN_LAYER_SIZE 25


int tile_width  = 64;
int tile_height = 60;

void sigmoid_matrix(float**, int, int, float**);
void add_bias_column_to_matrix(float **src_matrix, int rows, int cols, float **dest_matrix);
int *predict(float**, int, int, float**, int, int, float**, int, int);
int recognise_by_file(int, char**);
int recognise_by_pix_data(int input_examples, char *image_pixels);


int main (int argc, char *argv[])
{
    if(argc < 2)
    {
       printf("Give me at least one input file.\n");
       exit(-1);
    }

   char *fileName = argv[1];
   FILE *pFile;
   char var;
   unsigned i=0, ret=0;
   SDL_bool done = SDL_FALSE;
   int width=0, height=0, maxColorValue=0;
   char imageType[3];
   // Uint32 u32SmallTexPxls[tile_height*tile_width];
   //unsigned faces_found = 0;

   /* init SDL lib */
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
        printf("Error SDL init: %s.\n", SDL_GetError());
	return(-1);
   }
   /* initialize window to display the image */
   SDL_Window *win = SDL_CreateWindow("Drink more tea!", 100, 100, DISPLAY_WIN_WIDTH, DISPLAY_WIN_HEIGHT, SDL_WINDOW_SHOWN);
   if (win == NULL)
   {
	printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return(-1);
   }    

    /* renderer for rendering texture and rectangle */
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (ren == NULL)
    {
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
 

     /* Get the windows Surface */
     SDL_Surface *screen = SDL_GetWindowSurface(win);
     if(screen == NULL)
     {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        printf("Error GetWindowSurface: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
     }

     /* Create RGB masks of the image being displayed */
     Uint32 rmask = 0x000000FFU;
     Uint32 gmask = 0x0000FF00U;
     Uint32 bmask = 0x00FF0000U;
     Uint32 amask = 0x00000000U;

     /* Create surface for holding 64x60 tiles */
     SDL_Surface *tile_surface = SDL_CreateRGBSurface(0, tile_width, tile_height, 32, rmask, gmask, bmask, amask);

     if(tile_surface == NULL)
     {
         printf("Error creating tile surface: %s\n", SDL_GetError());
         SDL_FreeSurface(screen);
         SDL_DestroyRenderer(ren);
         SDL_DestroyWindow(win);
         SDL_Quit();
         return(-1);
     }

     /* Create texture to be rendered for viasualization of the captured image and box */
     SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, screen);
     SDL_FreeSurface(screen);
     SDL_Texture *tex_small = SDL_CreateTextureFromSurface(ren, tile_surface);
     //SDL_FreeSurface(tile_surface);

     if ( (tex == NULL) || (tex_small == NULL) )
     {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        printf("SDL_CreateTextureFromSurface Error: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
 
     /* Create new Rect obj to hold scaled image */
     SDL_Rect target_rect = { .x = 0, .y = 0, .w = tile_width, .h = tile_height };

     /* Rectangle to be displayed as a selection box */
     SDL_Rect bounding_box = { .x = 0, .y = 0, .w = tile_width, .h = tile_height };

     /* bounding box drawcolor */
     SDL_SetRenderDrawColor(ren, 250, 250, 0, SDL_ALPHA_OPAQUE); // opaque = 255, color = yellow
     
     /* Allocate memory for captured image pixels */
     char *pixels = (char *)malloc(DISPLAY_WIN_WIDTH*DISPLAY_WIN_HEIGHT*sizeof(char)); 
     if(pixels == NULL)
     {
        printf("Cannot allocate memory.\n");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return(-1);
     }


     /* allocate 64x60 matrix for holding extracted pixels */		
     char *current_tile = (char *)malloc(tile_height*tile_width*sizeof(char));
     if( current_tile == NULL )
     {
        printf("Error allocating memory for the current pixels tile block.\n");
	return(-1);
     }

     /* Event loop; interrupted by ESC key-press */
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
        SDL_Delay(25);

     /* Read captured .pgm image file, get type, weight, height, max color and pixel data */
     pFile = fopen(fileName, "rb");
     if(pFile == NULL)
     {
        printf("Cannot open %s.\n", fileName);
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
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
   

     if(ret == 0) // nothing has been red */
     {
         printf("Nothing has been red from the capture file.\n");
         SDL_DestroyRenderer(ren);
         SDL_DestroyWindow(win);
         SDL_Quit();
         return(-1);
     }

#if 0
     /* walk the image, asses the presense of a face */
     for(int i = 0; i < (height-tile_height); i+=10)
     {
 	for(int j = 0; j < (width-tile_width); j+=10)
	{
		for(int k = 0; k < tile_height; k++)
		{
			for(int m = 0; m < tile_width; m++)
			{
				char cp =  pixels[(i+k)*(width)+(j+m)];

				current_tile[k*tile_width + m] = cp;
				u32SmallTexPxls[k*tile_width + m] = (Uint32)(
                         (unsigned)0x00<<24 |
                         (unsigned)cp<<16 |
                         (unsigned)cp<< 8 |
                         (unsigned)cp);

				
			}
		}
		unsigned res = recognise_by_pix_data(1, current_tile);
		if( (res > 999) && (faces_found < FACES_FOUND_MAX) )
		{
			faces_found++;
			bbox_xy[faces_found][0] = j;
			bbox_xy[faces_found][1] = i;
		}
		else
		{
	//		break;
		}

		SDL_UpdateTexture(tex_small, NULL, (void*)u32SmallTexPxls, (4*tile_width));
		target_rect.x = j;
		target_rect.y = i;
		SDL_RenderCopy(ren, tex_small, NULL, &target_rect);
		SDL_RenderDrawRect(ren, &target_rect);
		SDL_RenderPresent(ren);	
		SDL_RenderCopy(ren, tex_small, NULL, &target_rect);
		SDL_RenderPresent(ren);	
		SDL_Delay(30);	
	}
     }
#endif

     unsigned int u32Pixels[width*height];
     unsigned int im_size = width*height;
  #pragma omp parallel default(none) private(i) shared(u32Pixels, pixels, im_size)
  {
     #pragma omp for
     for(int i = 0; i < im_size; i++)
     {
          u32Pixels[i] = (unsigned)(
                         (unsigned)0x00<<24 |
                         (unsigned)pixels[i]<<16 |
                         (unsigned)pixels[i]<< 8 |
                         (unsigned)pixels[i]);
     }
   } // end pragma omp parallel for

     int depth = 32;
     int pitch = 4*width;

     /* Construct surface from captured pixel data. This is working surface. */
     SDL_Surface *frame = SDL_CreateRGBSurfaceFrom((void*)u32Pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);
 
     if(frame == NULL)
     {
          printf("Error creating the frame: %s.\n", SDL_GetError());
          return(-1);
     }

     //SDL_SetSurfaceBlendMode(frame, SDL_BLENDMODE_NONE); 
     SDL_BlitScaled(frame, NULL, tile_surface, &target_rect);
     SDL_FreeSurface(frame);
     //SDL_BlitSurface(tile_surface, NULL, screen, &target_rect);

     SDL_UpdateTexture(tex, NULL, (void*)u32Pixels, pitch);
     SDL_UpdateTexture(tex_small, &target_rect, tile_surface->pixels, (4*tile_width));
     
     SDL_RenderClear(ren);
     /* Draw the texture */
     SDL_RenderCopy(ren, tex, NULL, NULL);
     SDL_RenderCopy(ren, tex_small, NULL, &target_rect);
      

     unsigned res = recognise_by_pix_data(1, tile_surface->pixels);
     if(res == 1)
     {
     	/* draw bounding boxes, where a face has been found */
        SDL_SetRenderDrawColor(ren, 250, 250, 0, SDL_ALPHA_OPAQUE); // opaque = 255, color = yellow
     	SDL_RenderDrawRect(ren, &bounding_box);
     }
     else
     {
	SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE); // opaque = 255, color = black
     	SDL_RenderDrawRect(ren, &bounding_box);

     }
  //   SDL_UpdateWindowSurface(win);
     
     /* Update the screen */
     SDL_RenderPresent(ren);

   
   } /* End of while(!done) */

   
    free(current_tile);
    free(pixels);
    
    SDL_FreeSurface(tile_surface);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;

// gcc -std=c99 -g -Wall display_jpg.c ann_file_ops.c -o display_jpg -DREENTRANT -I. -I/usr/include/SDL2 -lSDL2 -lSDL2_image
 
}

/** \fn int recognise_by_pix_data(int input_examples, char *image_pixels)
    \brief Wrapper function, performing prediction, based on input image raw pixels.

    \param[in] input_examples number of input image files.
    \param[in] image_list pointer to or array of image's pixels.
    \return the result of prediction. For now it is 1 for a face and 2 if not.
*/
int recognise_by_pix_data(int input_examples, char *image_pixels)
{
   float **image;
   int i, j, return_value = 1;
   
   image = allocate_matrix_floats(input_examples, INPUT_LAYER_SIZE, 0);
   if(image == NULL)
   {
      printf("Error allocating image.\n");
      return(-1);
   }

   for(i = 0; i < input_examples; i++)
   {

      /* Normalize pixes values in the range [-1, 1] */

#if(DEBUG_ON == 1)
      printf("Normalizing pixel data to [-1, 1] ... ");
#endif

   #pragma omp parallel default(none) shared(i, image, image_pixels) private(j)
   {
      #pragma omp for 
      for(j = 0; j < INPUT_LAYER_SIZE; j++)
      {
	   image[i][j] = (float)REMAP_PIXEL(image_pixels[j]);
      }
   } /* end pragma omp parallel */

#if(DEBUG_ON == 1)
      printf("Done.\n");
      print_matrix(image, input_examples, INPUT_LAYER_SIZE);
#endif
   }

   float **Theta1Copy, **Theta2Copy;

   Theta1Copy = allocate_matrix_floats(HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, 0); 
   Theta2Copy = allocate_matrix_floats(NUM_LABELS, HIDDEN_LAYER_SIZE+1, 0);
   if((Theta1Copy == NULL) || (Theta2Copy == NULL))
   {
       printf("Error allocating Theta copies.\n");
       return(-1);
   }

#if(DEBUG_ON == 1)
   printf("Copy Theta1 ... ");
#endif

 #pragma omp parallel shared(Theta1Copy) private(i, j)
 {
   #pragma omp for
   for(i=0; i < HIDDEN_LAYER_SIZE; i++)
   {
       for(j=0; j < INPUT_LAYER_SIZE+1; j++)
       {
           Theta1Copy[i][j] =  Theta1[i][j];	   
       }
   }
 } /* end of pragma omp parallel */

#if(DEBUG_ON == 1)
   printf("Done.\n");
#endif
#if(DEBUG_ON == 1)
   printf("Copy Theta2 ... ");
#endif

 #pragma omp parallel shared(Theta2Copy) private(i, j)
 {
   #pragma omp for
   for(i=0; i < NUM_LABELS; i++)
   {
       for(j=0; j < HIDDEN_LAYER_SIZE+1; j++)
       {
           Theta2Copy[i][j] =  Theta2[i][j];	   
       }
   }
 } /* end of pragma omp parallel */

#if(DEBUG_ON == 1)
   printf("Done.\n");
#endif
   
#if(DEBUG_ON == 1)
   printf("BEGIN predict ... \n");
#endif
   
   int *result = predict(Theta1Copy, HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, Theta2Copy, NUM_LABELS, HIDDEN_LAYER_SIZE+1, image, input_examples, INPUT_LAYER_SIZE);
   
#if(DEBUG_ON == 1)
   printf("Predict status: ... ");
#endif
   if(result != NULL)
   {     
#if(DEBUG_ON == 1)
       printf("OK.\n");
#endif
       return_value = result[0];
   }
   else
   {
      printf("Error.\n");
      return_value = -1;
   }

   int dealloc_status = 0;
#if(DEBUG_ON == 1)
   printf("Deallocate Thetas, image matrix, result ... ");
#endif
   dealloc_status = deallocate_matrix_floats(Theta1Copy, HIDDEN_LAYER_SIZE); 
   dealloc_status = deallocate_matrix_floats(Theta2Copy, NUM_LABELS);
   dealloc_status = deallocate_matrix_floats(image, input_examples);
   free(result);
#if(DEBUG_ON == 1)
   printf("Done - %s.\n", (dealloc_status?"Error!":"OK."));
#endif
   dealloc_status |= 1;
   return return_value;
}


/** \fn int recognise_by_file(int input_examples, char *image_list[])
    \brief Wrapper function, performing prediction, based on input image file.

    \param[in] input_examples number of input image files.
    \param[in] image_list pointer to or array of image file names.
    \return the result of prediction. For now it is 1 for a face and 2 if not.
*/
int recognise_by_file(int input_examples, char *image_list[])
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
      fileName = image_list[i];

      /* Read input image and store it in preallocated matrix */
#if(DEBUG_ON == 1)
      printf("Read file %s - %d of %d... ", fileName, i+1, input_examples);
#endif
      read_image_file(fileName, &imageData);
#if(DEBUG_ON == 1)
      printf("Done.\n");
#endif

//TODO check width, height, colorValue and adjust accordingly parameters, check dimentions!!!

#if(DEBUG_ON == 1)
      for(j=0; j < (imageData.width*imageData.height); j++)
      {
          printf("%f ", (imageData.paiPixels)[j]);
      }   
      printf("\n");
#endif

      /* Remap/scale pixes values in the range [-1, 1] */

#if(DEBUG_ON == 1)
      printf("Scaling pixel data to [-1, 1] ... ");
#endif

 #pragma omp parallel shared(i, image, imageData) private(j)
 {
   #pragma omp for
      for(j = 0; j < INPUT_LAYER_SIZE; j++)
      {
	   image[i][j] = (float)REMAP_PIXEL(imageData.paiPixels[j]);
      }
 } /* end of pragma omp parallel */

#if(DEBUG_ON == 1)
      printf("Done.\n");
      print_matrix(image, input_examples, INPUT_LAYER_SIZE);
#endif
#if(DEBUG_ON == 1)
      printf("Free image data pointer ...");
#endif
      free(imageData.paiPixels);
#if(DEBUG_ON == 1)
      printf("Done.\n");
#endif
   }

   float **Theta1Copy, **Theta2Copy;

   Theta1Copy = allocate_matrix_floats(HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, 0); 
   Theta2Copy = allocate_matrix_floats(NUM_LABELS, HIDDEN_LAYER_SIZE+1, 0);
   if((Theta1Copy == NULL) || (Theta2Copy == NULL))
   {
       printf("Error allocating Theta copies.\n");
       return(-1);
   }

#if(DEBUG_ON == 1)
   printf("Copy Theta1 ... ");
#endif
 
 #pragma omp parallel shared(Theta1Copy) private(i, j)
 {
   #pragma omp for
   for(i=0; i < HIDDEN_LAYER_SIZE; i++)
   {
       for(j=0; j < INPUT_LAYER_SIZE+1; j++)
       {
           Theta1Copy[i][j] =  Theta1[i][j];	   
       }
   }
 } /* end of pragma omp parallel */

#if(DEBUG_ON == 1)
   printf("Done.\n");
#endif
#if(DEBUG_ON == 1)
   printf("Copy Theta2 ... ");
#endif

 #pragma omp parallel shared(Theta2Copy) private(i, j)
 {
   #pragma omp for
   for(i=0; i < NUM_LABELS; i++)
   {
       for(j=0; j < HIDDEN_LAYER_SIZE+1; j++)
       {
           Theta2Copy[i][j] =  Theta2[i][j];	   
       }
   }
 } /* and of pragma omp parallel */

#if(DEBUG_ON == 1)
   printf("Done.\n");
#endif
   
#if(DEBUG_ON == 1)
   printf("BEGIN predict ... \n");
#endif
   
   int *result = predict(Theta1Copy, HIDDEN_LAYER_SIZE, INPUT_LAYER_SIZE+1, Theta2Copy, NUM_LABELS, HIDDEN_LAYER_SIZE+1, image, input_examples, INPUT_LAYER_SIZE);
   
#if(DEBUG_ON == 1)
   printf("Predict status: ... ");
#endif
   if(result != NULL)
   {     
#if(DEBUG_ON == 1)
       printf("OK.\n");
#endif
       for(i = 0; i < input_examples; i++)
       {
           printf("Image %s is %d.\n", image_list[i], (result[i]));
       }
       return_value = result[0];
   }
   else
   {
      printf("Error.\n");
      return_value = -1;
   }

   int dealloc_status = 0;
#if(DEBUG_ON == 1)
   printf("Deallocate Thetas, image matrix, result ... ");
#endif
   dealloc_status = deallocate_matrix_floats(Theta1Copy, HIDDEN_LAYER_SIZE); 
   dealloc_status = deallocate_matrix_floats(Theta2Copy, NUM_LABELS);
   dealloc_status = deallocate_matrix_floats(image, input_examples);
   free(result);
#if(DEBUG_ON == 1)
   printf("Done - %s.\n", (dealloc_status?"Error!":"OK."));
#endif
   dealloc_status |= 1;
   return return_value;
}


/** \fn void sigmoid_matrix(float **matrix, int rows, iment cols, float **result)
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
	
#pragma omp parallel shared(matrix, result) private(i, j)
 {
    #pragma omp for
    for(i=0; i < rows; i++)
    {
        for(j=0; j < cols; j++)
        {
             result[i][j] = (1.0 / (1.0 + exp(-matrix[i][j])));    
        }
    }
 } // end of parallel region
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
    \brief Predicts the label of an input given a trained neural network

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
   transpose_status |= 1; 
 
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
       printf("Cannot allocate memory for the result vector.\n");
       return(NULL);
   }
   for(i=0; i < XRows; i++)
   {
      if(h2Matrix[i][0] > h2Matrix[i][1]) // A Face
      {
          result[i] = 1;
#if(CHECK_LABELS == 1)
          printf("A Face at %d.", i);
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
          printf("\n");
#endif
      }
      else if(h2Matrix[i][0] == h2Matrix[i][1])
      {
          result[i] = 99;
#if(CHECK_LABELS == 1)
          printf("50/50 at %d.\n", i);
#endif
      }
      else
      {
          result[i] = 2;
#if(CHECK_LABELS == 1)
          printf("NOT a Face at %d.", i);
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
          printf("\n");
#endif
      }  
    
   }

#if(DEBUG_ON == 1)
   printf("Deallocate all local matrices ... ");
#endif
   dealloc_status += deallocate_matrix_floats(tempMatrix, Theta2Cols);
   dealloc_status += deallocate_matrix_floats(biasedMatrix, XRows);
   dealloc_status += deallocate_matrix_floats(h1Matrix, XRows);
   dealloc_status += deallocate_matrix_floats(h2Matrix, XRows);
#if(DEBUG_ON == 1)
   printf("Done - %s.\n", (dealloc_status?"Error!":"OK."));
#endif

#if(CHECK_LABELS == 1) 
   printf("True Positives: %d.\nTrue Negatives: %d.\nFalse Positives: %d.\nFlase Negatives: %d.\n",\
          truePositives, trueNegatives, falsePositives, falseNegatives);
#endif

   return(result);
}

/** \fn int sigmoid_gradient_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
 *  \brief Computes the gradient of the sigmoid function, evaluated at input parameter
 *
 *  dest_mx = sigmoid(src_mx).*(1 - sigmoid(src_mx));
 *
 *  \param[in] **src_matrix
 *  \param[in] rows the rows count of the input/src matrix
 *  \param[in] cols the columns count of the input/src matrix
 *  \param[out] **dest_matrix the pointer th the output matrix
 *  \return zero on success, -1 otherwise.
 */

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
   dealloc_status |= 1;
   return(0);
}

/** \fn int nnCostFunction(float **Theta1, float **Theta2, int input_layer_size, int hidden_layer_size, int num_labels, int input_examples, float **X, float **y, float lambda, float *J, float **gradient)
 * \brief Computes the cost and the gradient of the ANN.
 *
 * \param[in] a lot
 * \param[out] J
 * \param[out] **gradient
 * \return nothing
 */

int nnCostFunction(float **Theta1, float **Theta2, int input_layer_size, int hidden_layer_size, int num_labels, int input_examples, float **X, float **y, float lambda, float *J, float **gradient)
{
    int i, j, mult_status, transpose_status;
    float **a1 = allocate_matrix_floats(input_examples, input_layer_size+1, 0);
    float **z2 = allocate_matrix_floats(input_examples, hidden_layer_size, 0);
    float **Theta1TR = allocate_matrix_floats(hidden_layer_size, input_layer_size+1, 0);

    /* input matrix calculations */
    add_bias_column_to_matrix(X, input_examples, input_layer_size+1, a1);
    transpose_status = transpose_matrix(Theta1, hidden_layer_size, input_layer_size+1, Theta1TR);
    if(transpose_status != 0)
    {
        printf("Error in transposing Theta1'.\n");
        return(-1);
    }

    mult_status = ALG_MATMUL2D(input_examples, hidden_layer_size, input_layer_size+1, a1, Theta1TR, z2);
    if(mult_status != 0)
    {
        printf("Error in multiply a1 * Theta1'.\n");
        return(-1);
    }
    /* free some memory for the next stage */
    deallocate_matrix_floats(Theta1TR, hidden_layer_size);

    float **a2 = allocate_matrix_floats(input_examples, hidden_layer_size, 0);
    float **a2_biased = allocate_matrix_floats(input_examples, hidden_layer_size+1, 0);

    /* 1-st hidden layer calculation */
    sigmoid_matrix(z2, input_examples, hidden_layer_size, a2);
    add_bias_column_to_matrix(a2, input_examples, hidden_layer_size+1, a2_biased);

    deallocate_matrix_floats(a2, input_examples);
    deallocate_matrix_floats(z2, input_examples);

    float **z3 = allocate_matrix_floats(input_examples, num_labels, 0);
    float **Theta2TR = allocate_matrix_floats(num_labels, hidden_layer_size+1, 0);
    
    /* output layer, i.e. hypothesis calculations */
    transpose_status = transpose_matrix(Theta2, num_labels, hidden_layer_size+1, Theta2TR);
    if(transpose_status != 0)
    {
        printf("Error in transposing Theta2'.\n");
        return(-1);
    }
    
    mult_status = ALG_MATMUL2D(input_examples, num_labels, hidden_layer_size+1, a2_biased, Theta2TR, z3);
    if(mult_status != 0)
    {
        printf("Error in multiply a2 * Theta2'.\n");
        return(-1);
    }

    float **a3 = allocate_matrix_floats(input_examples, num_labels, 0);
    sigmoid_matrix(z3,input_examples, num_labels, a3);

    /* free some memory for the next stage */
    deallocate_matrix_floats(Theta2TR, hidden_layer_size);
    deallocate_matrix_floats(z3, hidden_layer_size);

    /* calculate cost */
    i = 2; j = 2; i += j;

    return(0);
}
