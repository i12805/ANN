#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define WIDTH 320
#define HEIGHT 240


int main(int argc, char * argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error init: %s.\n", SDL_GetError());
	return -1;
    }

	IMG_Init(IMG_INIT_JPG);

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
	printf("SDL_CreateWindow Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return -1;
    }    
/*
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == NULL)
    {
	SDL_DestroyWindow(win);
	printf("SDL_CreateRenderer Error: %s", SDL_GetError());
	SDL_Quit();
	return -1;
     }
  */   
//     char *path_to_image = "images/test0.jpeg";
/*
     SDL_Surface *bmp = SDL_LoadBMP(argv[1]);
     if (bmp == NULL)
     {
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	printf("SDL_LoadBMP Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return -11;
     }
*/

     SDL_Surface *screen = SDL_GetWindowSurface(win);
     if (screen == NULL)
     {
	//SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	printf("SDL_GetWindowSurface screen  Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return(-1);
     }
/*
     SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, screen);
     SDL_FreeSurface(screen);
     if (tex == NULL)
     {
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	printf("SDL_CreateTextureFromSurface Error: %s.\n", SDL_GetError());
	SDL_Quit();
	return -1;
     }
*/



/******************* Video *********/

	int fd;
	char *video_dev = "/dev/video0";
	if((fd = open(video_dev, O_RDWR)) < 0)
	{
		printf("Error open %s.\n", video_dev);
		return(-1);
	}

	/* Retrieve the device’s capabilities */
	struct v4l2_capability cap;
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
	{
	    perror("VIDIOC_QUERYCAP");
	    return(-1);
	}
	
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf(stderr, "The device does not handle single-planar video capture.\n");
		return(-1);
	}

	/* set video format */
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	format.fmt.pix.width = 64;
	format.fmt.pix.height = 60;
	 
	if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
	    printf("Error VIDIOC_S_FMT.\n");
	    return(-1);
	}

	/* set data buffers */
	struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 1;
	
	/* map memory to the buffer */
	if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0)
	{
	    printf("Error VIDIOC_REQBUFS.\n");
	    return(-1);
	}
	
	/* allocate buffer */
	struct v4l2_buffer bufferinfo;
	memset(&bufferinfo, 0, sizeof(bufferinfo)); //clear memory unde the buffer
 
	bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufferinfo.memory = V4L2_MEMORY_MMAP;
	bufferinfo.index = 0;
 
	if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0)
	{
	    printf("Error VIDIOC_QUERYBUF.\n");
	    return(-1);
	}

	void* buffer_start = mmap( NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufferinfo.m.offset);
 
	if(buffer_start == MAP_FAILED){
	    perror("mmap");
	    return(-1);
	}
 
	memset(buffer_start, 0, bufferinfo.length); // clear the area


	//struct v4l2_buffer bufferinfo;
	//memset(&bufferinfo, 0, sizeof(bufferinfo));
 
	//bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//bufferinfo.memory = V4L2_MEMORY_MMAP;
	//bufferinfo.index = 0; /* Queueing buffer index 0. */
 
	// Activate streaming
	//int type = bufferinfo.type;
	//if(ioctl(fd, VIDIOC_STREAMON, &type) < 0)
//	{
//	    perror("VIDIOC_STREAMON");
//	    exit(1);
//	}

	SDL_RWops *buffer_stream;
	SDL_Surface *frame;
	SDL_bool done = SDL_FALSE;
	struct v4l2_buffer buf;

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

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;		 
		buf.index = 0;
		// Put the buffer in the incoming queue.
		if(ioctl(fd, VIDIOC_QBUF, &buf) < 0){
		    perror("VIDIOC_QBUF");
		    exit(1);
		}
		 
	// Activate streaming
	int type = bufferinfo.type;
	if(ioctl(fd, VIDIOC_STREAMON, &type) < 0)
	{
	    perror("VIDIOC_STREAMON");
	    exit(1);
	}
		// The buffer's waiting in the outgoing queue.
		if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
		    perror("VIDIOC_DQBUF");
		    exit(1);
		}
#if(0)
		buffer_stream = SDL_RWFromMem(buffer_start, bufferinfo.length);
		frame = IMG_Load_RW(buffer_stream, 0);
#else
		buffer_stream = SDL_RWFromFile("images/test.pgm", "rb");
		frame = IMG_LoadPNM_RW(buffer_stream);
#endif
		if(frame == NULL)
		{
			printf("Error loading the frame: %s.\n", IMG_GetError());
			return(-1);
		}
		SDL_BlitSurface(frame, NULL, screen, NULL);
	//	SDL_Flip(screen);

		//First clear the renderer
		//SDL_RenderClear(ren);
		//Draw the texture
		//SDL_RenderCopy(ren, tex, NULL, NULL);
		//Update the screen
		//SDL_RenderPresent(ren);
		//Take a quick break after all that hard work
		//SDL_Delay(1000);
		
		SDL_UpdateWindowSurface(win);
		 
	/* Your loops end here. */
	} 
	// Deactivate streaming
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
	{
	    perror("VIDIOC_STREAMOFF");
	    exit(1);
	}

	close(fd);

      	SDL_FreeSurface(frame);
	SDL_FreeSurface(screen);
	SDL_RWclose(buffer_stream);      

  //   SDL_DestroyTexture(tex);
  //   SDL_DestroyRenderer(ren);
     SDL_DestroyWindow(win);
     SDL_Quit();

    return 0;
}


//-std=c99 -g -Wall new.c -o sdl -D_REENTRANT -I/usr/include/SDL2 -lSDL2 -lSDL2_image
