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

#endif
