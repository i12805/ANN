#ifndef ANN_FILE_OPS_H
#define ANN_FILE_OPS_H


typedef struct pgm_image_t 
{
   char imageType[3];
   int width;
   int height;
   int maxColorValue;
   float *paiPixels;
} pgm_image_t;


int read_image_file(char*, pgm_image_t*);
int display_image_file(char *path_to_image, int width, int height);
int display_image_mem(void *pixels, int width, int height);
pgm_image_t read_pgm_binary(char *fileName);

#endif
