#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include<unistd.h>

#include<sys/ioctl.h>

#include<sys/types.h>

#include<sys/stat.h>

#include<fcntl.h>

#include<sys/select.h>

#include<sys/time.h>

#include<errno.h>

#include <time.h>

typedef struct{
    int w,h,c;
    float *data;
} image;

typedef struct{
    float x, y;
} point;

image make_empty_image(int w, int h, int c)
{
    image out;
    out.data = 0;
    out.h = h;
    out.w = w;
    out.c = c;
    return out;
}

image make_image(int w, int h, int c)
{
    image out = make_empty_image(w,h,c);
    out.data = calloc(h*w*c, sizeof(float));
    return out;
}

image load_image_stb(char *filename, int channels)
{
    int w, h, c;
    unsigned char *data = stbi_load(filename, &w, &h, &c, channels);
    if (!data) {
        fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n",
            filename, stbi_failure_reason());
        exit(0);
    }
    if (channels) c = channels;
    int i,j,k;
    image im = make_image(w, h, c);
    for(k = 0; k < c; ++k){
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                int dst_index = i + w*j + w*h*k;
                int src_index = k + c*i + c*w*j;
                im.data[dst_index] = (float)data[src_index]/255.;
            }
        }
    }
    //We don't like alpha channels, #YOLO
    if(im.c == 4) im.c = 3;
    free(data);
    return im;
}

image load_image(char *filename)
{
    image out = load_image_stb(filename, 0);
    return out;
}

void pad_index(int* in, int lim)
{
    if (*in < 0)
    {
        *in = 0;
    }
    else if (*in >= lim)
    {
        *in = lim - 1;
    }
}

float get_pixel(image im, int x, int y)
{
    int c = 0;
    int max_ind = im.w * im.h * im.c - 1;
    //printf("w=%d h=%d c=%d\n", im.w, im.h, im.c);
    pad_index(&c, im.c);
    pad_index(&x, im.w);
    pad_index(&y, im.h);
    int ind = c * im.w * im.h + y * im.w + x;
    //printf("padded: w=%d h=%d c=%d\n", x, y, c);
    if (ind < 0)
    {
        ind = 0;
    }
    else if (ind > max_ind)
    {
        ind = max_ind;
    }
    else
    {
        // index is valid
    }

    return im.data[ind];
}

int main(int argc, char** argv) 
{
   image im = load_image(argv[1]);	
 
   int width = im.w;
   int heigh = im.h;
   int i, j, k;
   float s = 0;
   float sq[width][heigh];

   for (i=0;i<width;i++)
   {
      for (j=0;j<heigh;j++)
      {
         sq[i][j] = get_pixel(im, i, j);
	 s += sq[i][j];
      }
   }

   s=0.95 * s/ (width * heigh) ;

   char left[1] = "l";
   char right[1] = "r";
   char up[1] = "u";
   char down[1] = "d";
   char click[1] = "q";

   int fd = open("/dev/mousek", O_RDWR);
   struct timespec tim, tim2;
   tim.tv_sec = 0;
   tim.tv_nsec = 12000000L;

   struct timespec tim3, tim4;
   tim3.tv_sec = 0;
   tim3.tv_nsec = 120000L;

   for (j=0;j<heigh;j+=2)
   {
      nanosleep(&tim , &tim2);
      write(fd, &down, 4);
      for (i=0;i<width;i++)
      {
	if ((sq[i][j] < s))
	{
	   write(fd, &click, 4);
	}
	nanosleep(&tim , &tim2);
	write(fd, &right, 4);
      }
      nanosleep(&tim , &tim2);
      write(fd, &down, 4);
      for (i=0;i<width;i++)
      {
	if ((sq[width-i-1][j+1] < s))
	{
	   write(fd, &click, 4);
	}
	nanosleep(&tim , &tim2);
	write(fd, &left, 4);
      }
   }

   close(fd);
   return 0;
}
