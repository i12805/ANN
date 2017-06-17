#include <stdio.h>
#include <stdlib.h>
#include "ann_matrix_ops.h"

#ifdef USE_PARALLELLA
#include <e-hal.h>
#else
#include <omp.h>
#endif
/* Matrix manipulation functions */

/* Matrix aux. functions for char */

/** \fn char **allocate_matrix_chars(int rows, int cols, char epsilon_init)
    \brief Allocate memory rows x cols of chars and initialises the matrix

    If parameter epsilon_init is not zero, the init valie of the matix member would be rand()*2*epsilon - epsilon.

    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[in] epsilon_init init factor.
    \return a pointer to the new matrix. In failure, returns NULL pointer.
*/
char **allocate_matrix_chars(int rows, int cols, char epsilon_init)
{
   int i, j, kur;
   char **matrix, a;
   
   matrix = (char **)malloc(rows*sizeof(char*));
   if(matrix == NULL)
   {
      printf("ERROR allocating memory for rows of a matrix %dx%d.\n", rows, cols);
      return(NULL);
   }
   for(i=0; i < rows; i++)
   {
       matrix[i] = (char*)malloc(cols*sizeof(char));
       if(matrix[i] == NULL)
       { 
            printf("ERROR allocating memory for cols of a matrix %dx%d.\n", rows, cols);
            kur = deallocate_matrix_chars(matrix, rows);
            printf("Deallocation status is %d.\n", kur);   
            return(NULL);
       }

       /* initializes the matrix with random values or zeros */
       for (j = 0; j < cols; j++)
       {
           a = rand();
           matrix[i][j] = (a*2*epsilon_init - epsilon_init);
       }
   }
   return(matrix);
}

/** \fn intr **deallocate_matrix_chars(char **matrix, int rows)
    \brief Deallocate memory used for matrix of chars

    \param[in] **matrix a pointer to the 2D matrix to be wiped out.
    \param[in] rows number of rows of the matrix.
    \return zero on success, -1 otherwise.
*/
int deallocate_matrix_chars(char **matrix, int rows)
{
   int i;

   if(matrix == NULL)
   {
       return(-1);
   }

   for(i=0; i < rows; i++)
   {
       free(matrix[i]);
       matrix[i] = NULL;
   }
   free(matrix);
   matrix = NULL;
   return(0);
}


/** \fn int transpose_matrix(char **src_matrix, int rows, int cols, char **dest_matrix)
    \brief Transpose a 2D matrix.

    \param[in] **src_matrix  pointer to the 2D source matrix.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[out] **dest_matrix  pointer to the 2D destination(i.e. transposed) matrix.
    \return zero in success, -1 if any of src or dest pointers are null.
*/

int transpose_matrix_chars(char **src_matrix, int rows, int cols, char **dest_matrix)
{
   
   int i, j;
   if((src_matrix == NULL) || (dest_matrix == NULL))
   {
       return(-1);
   }

#pragma omp parallel private(i, j) shared(dest_matrix, src_matrix, rows, cols)
 {
   #pragma omp for schedule(static)
   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     dest_matrix[j][i] = src_matrix[i][j];
    }
   }
 } // end of parallel region
   return(0);
}


void print_matrix_char(char **matrix, int rows, int cols)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     printf("%d ",matrix[i][j]);
    }
    printf("\n");
   }
   putchar('\n');
}



/* **************** Matrix aux functions for floats **************** */

/** \fn float **allocate_matrix_floats(int rows, int cols, float epsilon_init)
    \brief Allocate memory rows x cols of floats and initialises the matrix

    If parameter epsilon_init is not zero, the init valie of the matix member would be rand()*2*epsilon - epsilon.

    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[in] epsilon_init init factor. typical value = 0.12
    \return a pointer to the new matrix. In failure, returns NULL pointer.
*/
float **allocate_matrix_floats(int rows, int cols, float epsilon_init)
{
   int i, j, kur;
   float **matrix, a;
   
   matrix = (float **)malloc(rows*sizeof(float*));
   if(matrix == NULL)
   {
      printf("ERROR allocating memory for rows of a matrix %dx%d.\n", rows, cols);
      return(NULL);
   }
   for(i=0; i < rows; i++)
   {
       matrix[i] = (float*)malloc(cols*sizeof(float));
       if(matrix[i] == NULL)
       { 
            printf("ERROR allocating memory for cols of a matrix %dx%d.\n", rows, cols);
            kur = deallocate_matrix_floats(matrix, rows);
            printf("Deallocation status is %d.\n", kur);   
            return(NULL);
       }

       /* initializes the matrix with rsndom values or zeros */
       for (j = 0; j < cols; j++)
       {
           a = rand();
           matrix[i][j] = (a*2.0*epsilon_init - epsilon_init);
       }
   }
   return(matrix);
}

int deallocate_matrix_floats(float **matrix, int rows)
{
   int i;

   if(matrix == NULL)
   {
       return(-1);
   }

   for(i=0; i < rows; i++)
   {
       free(matrix[i]);
       matrix[i] = NULL;
   }
   free(matrix);
   matrix = NULL;
   return(0);
}

int init_matrix(float **matrix, int rows, int cols, float init_value)
{
   int i, j;
   if(matrix == NULL)
   {
      return(-1);
   }

   for(i=0; i < rows; i++)
   {
      for(j=0; j < cols; j++)
      {
         matrix[i][j] = init_value;
      }
   }
   return(0);
}

void print_matrix(float **matrix, int rows, int cols)
{
   int i, j;

   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     printf("%.5f ", matrix[i][j]);
    }
    putchar('\n');
   }
   putchar('\n');
}


/** \fn int transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
    \brief Transpose a 2D matrix.

    \param[in] **src_matrix  pointer to the 2D source matrix.
    \param[in] rows number of rows of the matrix.
    \param[in] cols number of columns of the matrix.
    \param[out] **dest_matrix  pointer to the 2D destination(i.e. transposed) matrix.
    \return zero in success, -1 if any of src or dest pointers are null.
*/

int transpose_matrix(float **src_matrix, int rows, int cols, float **dest_matrix)
{
   
   int i, j;
   if((src_matrix == NULL) || (dest_matrix == NULL))
   {
       return(-1);
   }

#pragma omp parallel private(i, j) shared(dest_matrix, src_matrix, rows, cols)
 {
   #pragma omp for schedule(static)
   for(i=0; i < rows; i++)
   {
    for(j=0; j < cols; j++)
    {
     dest_matrix[j][i] = src_matrix[i][j];
    }
   }
 } // end of parallel region
   return(0);
}

/** \fn int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C)
    \brief Multiplies two 2D matrices of floats.

    \param[in] M number of rows of the first multiplier matrix.
    \param[in] N number of columns of the second multiplier matrix.        
    \param[in] P number of columns of the first multiplier, resp. rows of the first multipl.  matrix.
    \param[in] **A pointer to the first miltiplier - a 2D matrix of floats.
    \param[in] **B pointer to the second multiplier - a 2D matrix of floats.
    \param[out] **C pointer to the result (2D) matrix of floats.
    \return zero on success, -1 otherwise.
*/
#ifdef USE_PARALLELLA
int ALG_MATMUL2D(int M, int N, int K, float** A, float** B, float** C)
{
    int cores_row, cores_col, i, j, k, return_value = 1;
    unsigned clr = 0, section, CORES = 16;
    e_platform_t platform;
    e_epiphany_t dev;

    section = (unsigned)(K/CORES);
    unsigned remaining_elements = (unsigned)(K%CORES);

    /* Init Epiphany device */
    e_init(NULL);
    /* Reset Epiphany */
    e_reset_system();
    e_get_platform_info(&platform);
    /* open group or all cores - all cores here */
    if(e_open(&dev, 0, 0, platform.rows, platform.cols))
    {
        printf("\nERR Cannot connect to Epiphany device.\n");
        return(-1);
    }
    /* Load kernel program to the cores (all of them), postpone execution */
    if(e_load_group("e_task.elf", &dev, 0, 0, platform.rows, platform.cols, E_FALSE))
    {
        printf("\nERR Cannot load the kernel to Epiphany device.\n");
        return(-1);
    }


    for(i = 0; i < M; i++)
    {
        for(j = 0; j < N; j++)
        {
            float c_temp = 0.0;
            /* next code calculates one K-th element */
           
            /* Copy data from host to Epiphany local memory and clear "Done" flag for every core.*/
            for(cores_row = 0; cores_row < platform.rows; cores_row++)
            {
                for(cores_col = 0; cores_col < platform.cols; cores_col++)
                {
                    e_write(&dev, cores_row, cores_col, 0x2000, A[i][(cores_row*platform.cols+cores_col)*section], sizeof(float)*section);
                    e_write(&dev, cores_row, cores_col, 0x4000, B[(cores_row*platform.cols+cores_col)*section][j], sizeof(float)*section);
                    e_write(&dev, cores_row, cores_col, 0x7000, &clr, sizeof(clr));
                    e_write(&dev, cores_row, cores_col, 0x7004, &section, sizeof(section));
                    e_write(&dev, cores_row, cores_col, 0x7008, &CORES, sizeof(CORES));
                }
            }
            /* Start cores */
            e_start_group(&dev);
            /* Check if all cores are finished */
            unsigned all_done = 0, done[CORES];
            while(all_done < CORES)
            {
                for(cores_row = 0; cores_row < platform.rows; cores_row++)
                {
                    for(cores_col = 0; cores_col < platform.cols; cores_col++)
                    {
                        e_read(&dev, cores_row, cores_col, 0x7000, &done[cores_row*platform.cols+cores_col], sizeof(unsigned));
                        all_done += done[cores_row*platform.cols+cores_col];
                    }
                }
            }
            for(cores_row = 0; cores_row < platform.rows; cores_row++)
            {
                for(cores_col = 0; cores_col < platform.cols; cores_col++)
                {
                    e_read(&dev, cores_row, cores_col, 0x6000, &c_temp, sizeof(float));
                    C[i][j] += c_temp;
                    //   /* CHeCK CORE COMMINICATION */
                    //   unsigned coreid = 99;
                    //   e_read(&dev, cores_row, cores_col, 0x700C, &coreid, sizeof(unsigned));
                    //   printf("Checking core (%d, %d) 0x%03X.\n", cores_row, cores_col, coreid);

                }
            }
            for(k=((CORES*section)-1); k < K; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
            /* end of K-th element calc */
        }
    }
    /* Close Epiphany */
    e_close(&dev);
    e_finalize();
    return_value = 0;

    return(return_value);
}

#else
/* on the ZYNQ/BRCM ARM Cortex A9 (or whatever) use openmp approach */
int ALG_MATMUL2D(int M, int N, int P, float** A, float** B, float** C)
{
   int I=0, J, K;
   float temp;
 
 #pragma omp parallel shared(A,B,C, M, N, P) private(I,J,K,temp)
 {
	J = 0; K = 0; temp = 0.0;
	#pragma omp parallel for schedule(static)
	//#pragma omp parallel for private(J,K,temp)
		 
   		for (I=0; I<M; I++)
   		{
      			for (J=0; J<N; J++)
      			{
         			//C[I][J]=0.;
         			temp = 0.0;
         			for (K=0; K<P; K++)
         			{	
            				//C[I][J]=(C[I][J])+((A[I][K])*(B[K][J]));
			            	temp += A[I][K] * B[K][J];
        			 }
      				C[I][J] = temp;
      			}
  	 	}	
 	
 }
 return(0);
}
#endif
