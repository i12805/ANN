#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
#include "ann_file_ops.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>


typedef struct buffer {
        void   *start;
        size_t length;
}sV4L2Buffer_t;

#define WIDTH 640
#define HEIGHT 480

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct v4l2_format              fmt;
struct v4l2_buffer              buf;
struct v4l2_requestbuffers      req;
enum v4l2_buf_type              type;
fd_set                          fds;
struct timeval                  tv;
volatile int                    r, fd = -1;
unsigned int                    i, n_buffers;
char                            *dev_name = "/dev/video0";
char                            out_name[256];
FILE                            *fout;
sV4L2Buffer_t                   *buffers;
sV4L2Buffer_t                   *buffers_copy;
static volatile unsigned int    u32BytesUsed = 0;

Uint32 u32Pixels[WIDTH*HEIGHT];
SDL_bool done = SDL_FALSE;
SDL_Window *win = NULL;
SDL_Renderer *ren = NULL;
SDL_Texture *tex = NULL;

#if defined(USE_YUV2)
SDL_PixelFormat spf;
SDL_PixelFormat *psYUV2PixelFormat = &spf;

Uint8 * pu8R = 0;
Uint8 * pu8G = 0;
Uint8 * pu8B = 0;
#endif

/** @fn static void xioctl(int fh, int request, void *arg)
 *  @brief ioctl wrap function
 *  @param[in] int fh device handler
 *  @param[in] int request request enum
 *  @param[in] void * arg input parameters pointer
 *  @return void
 */
static void xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

/** @fn void vCaptureInit()
 *  @brief Initialize video capture, using V4L2.
 *  @return void
 */
void vCaptureInit()
{
        fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0) {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = 640;
        fmt.fmt.pix.height      = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; //V4L2_PIX_FMT_RGB24;
       // fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) {
                printf("Libv4l didn't accept PIX format. %X .\n", fmt.fmt.pix.pixelformat);
                //exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
                printf("Warning: driver is sending image at %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

        CLEAR(req);
        req.count = 1;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = calloc(req.count, sizeof(*buffers));
        buffers_copy = calloc(req.count, sizeof(*buffers));
        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                xioctl(fd, VIDIOC_QUERYBUF, &buf);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start) {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }
}

/** @fn int s32Capture(unsigned int, struct buffer *)
 *  @brief Captures some frames from the usb camera, attached
 *  @param[in] unsigned int number of frames
 *  @param[out] a pointer to the pixel data holder
 *  @param[out] a pointer to the data length holder
 *  @return zero if OK, -1 if error
 */
int s32Capture(unsigned int u32Frames)
{
        for (i = 0; i < n_buffers; ++i) {
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                xioctl(fd, VIDIOC_QBUF, &buf);
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        xioctl(fd, VIDIOC_STREAMON, &type);
        for (i = 0; i < u32Frames; i++) {
                do {
                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select(fd + 1, &fds, NULL, NULL, &tv);
                } while ((r == -1 && (errno = EINTR)));
                if (r == -1) {
                        perror("select");
                        return errno;
                }

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                xioctl(fd, VIDIOC_DQBUF, &buf);

                sprintf(out_name, "images/out%03d.ppm", i);
                fout = fopen(out_name, "w");
                if (!fout) {
                        perror("Cannot open image");
                        exit(EXIT_FAILURE);
                }
                fprintf(fout, "P6\n%d %d 255\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
                fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
                u32BytesUsed = (unsigned int)buf.bytesused;
                printf("buf inside capture fn: %d\n", u32BytesUsed);
                memcpy(buffers_copy, buffers[0].start, u32BytesUsed);
                fclose(fout);
                
                xioctl(fd, VIDIOC_QBUF, &buf);
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);

        return 0;
}

/** @fn void vCaptureClose()
 *  @brief Close video devise handler and unmap memory capture buffers
 *  @return zero if OK, -1 if error
 */
void vCaptureClose()
{
        for (i = 0; i < n_buffers; ++i)
                munmap(buffers[i].start, buffers[i].length);
        close(fd);
}


int iSDL_Init(void)
{
   /* initialize window to display the image */
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
        printf("Error SDL init: %s.\n", SDL_GetError());
        return(-1);
   }
   win = SDL_CreateWindow("Drink more tea!", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
   if (win == NULL)
   {
        printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
   }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (ren == NULL)
    {
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
     
#if defined(USE_YUV2)
     tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
#else
     tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
#endif

     if (tex == NULL)
     {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        printf("SDL_CreateTextureFromSurface Error: %s.\n", SDL_GetError());
        SDL_Quit();
        return(-1);
     }
#if defined(USE_YUV2)
    psYUV2PixelFormat->format=SDL_PIXELFORMAT_YUY2;
    psYUV2PixelFormat->palette=NULL;
    psYUV2PixelFormat->BitsPerPixel=16;
    psYUV2PixelFormat->BytesPerPixel=2;
    psYUV2PixelFormat->Rmask=(Uint32)0x000000FFU;
    psYUV2PixelFormat->Gmask=(Uint32)0x0000FF00U; 
    psYUV2PixelFormat->Bmask=(Uint32)0x00FF0000U;
    psYUV2PixelFormat->Amask=(Uint32)0xFF000000U;
#endif
       
#if defined(USE_BOUNDING_BOX)
     //SDL_Rect bounding_box = { .x=10, .y=10, .w=64, .h=60 };
     //SDL_SetRenderDrawColor(ren, 250, 250, 0, SDL_ALPHA_OPAQUE); // opaque = 255
#endif
     return(0);
 }

void vSDL_Close(void)
{
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    //char *fileName = argv[1];
    printf("Enter main.\n");
    vCaptureInit();
    int res = iSDL_Init();
    if(res !=0)
    {
        printf("Error in SDL init. Quit.\n");
        return(1);
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

        /* Capture camera frame */
        int capres = s32Capture(1);
        if( (capres != 0) )
        {
            printf("Capturing camera frame failed.\n");
            vCaptureClose();
            SDL_DestroyTexture(tex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return(-1);
        }

        printf("Bytes used in capture buffer: %d.\n", u32BytesUsed);
        SDL_Delay(50);

        /* Get pixel data */
         Uint32 *video_buffer_data = (Uint32 *)(buffers_copy->start);
        printf("raw buffer: 0x%08X\n", *video_buffer_data);
        int i;
        for(i=0; i < u32BytesUsed/2; i+=2)
        {
        
        #if defined(USE_YUV2)
            SDL_GetRGB( (Uint32)(*video_buffer_data), psYUV2PixelFormat, pu8R, pu8G, pu8B);
            u32Pixels[i] = (unsigned)0x00<<24 |
                           (unsigned)(*pu8R)<<16 |
                           (unsigned)(*pu8G)<< 8 |
                           (unsigned)(*pu8B);
            //SDL_UpdateYUVTexture(tex, NULL, u8YPlane, WIDTH, u8UPlane, 4*WIDTH, u8VPlane, 4*WIDTH);
        #else
            Uint8 Y1 = (*(video_buffer_data)) >> 24;
            Uint8 Cb = ((*video_buffer_data) & 0x00FF0000U) >> 16;
            Uint8 Y2 = ((*video_buffer_data) & 0x0000FF00U) >> 8;
            Uint8 Cr = ((*video_buffer_data) & 0x000000FFU);
            
            u32Pixels[i] = (unsigned)0x00<<24 |
                           (unsigned)(Y1+1.402*Cr)<<16 |
                           (unsigned)(Y1-0.344*Cb-0.714*Cb)<< 8 |
                           (unsigned)(Y1+1.772*Cb);
            
            u32Pixels[i+1] = (unsigned)0x00<<24 |
                           (unsigned)(Y2+1.402*Cr)<<16 |
                           (unsigned)(Y2-0.344*Cb-0.714*Cb)<< 8 |
                           (unsigned)(Y2+1.772*Cb);
         #endif    
           video_buffer_data++;
        }
   
    #if defined(USE_BOUNDING_BOX)
        bounding_box.y = 0;
        bounding_box.x = 0;
    #endif
        int tex_upd = SDL_UpdateTexture(tex, NULL, (void*)u32Pixels, 4*WIDTH);        
        if (tex_upd != 0)
        {
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
            printf("SDL_CreateTextureFromSurface Error: %s.\n", SDL_GetError());
            SDL_Quit();
            return(-1);
        }
        SDL_RenderClear(ren);
        /* Draw the texture */
        SDL_RenderCopy(ren, tex, NULL, NULL);
        /* Draw the bounding box */
    #if defined(USE_BOUNDING_BOX)
        SDL_RenderDrawRect(ren, &bounding_box);
    #endif
        /* Update the screen */
        SDL_RenderPresent(ren);
   
    } /* End of while(!done) */

    #if defined(USE_YUV2)
    //free(u8YPlane);
    //free(u8UPlane);
    //free(u8VPlane);
    #endif
    vCaptureClose();
    vSDL_Close();
    
    return 0;
}
// gcc -std=c99 -g -Wall display_jpg.c ann_file_ops.c -o display_jpg -D_REENTRANT -I. -I/usr/include/SDL2 -lSDL2 -lSDL2_image
