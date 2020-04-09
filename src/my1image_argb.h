/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_ARGBH__
#define __MY1IMAGE_ARGBH__
/*----------------------------------------------------------------------------*/
#include "my1image_gray.h"
/*----------------------------------------------------------------------------*/
/* image color (rgb) stuffs */
#define IMASK_GRAY    0x00000000
#define IMASK_COLOR   0x00FFFFFF
#define IMASK_COLOR_R 0x00FF0000
#define IMASK_COLOR_G 0x0000FF00
#define IMASK_COLOR_B 0x000000FF
#define IMASK_COLOR_A 0xFF000000
/*----------------------------------------------------------------------------*/
typedef unsigned char cbyte; /** color byte */
/*----------------------------------------------------------------------------*/
typedef struct
{
	cbyte b,g,r,a;
}
my1image_rgb_t;
/*----------------------------------------------------------------------------*/
typedef my1image_rgb_t my1rgb_t;
/*----------------------------------------------------------------------------*/
/* rgb conversion utility */
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b);
int encode_rgb(cbyte r, cbyte g, cbyte b);
int gray4rgb(cbyte r, cbyte g, cbyte b);
int gray2color(int data);
int color2gray(int data);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_ARGBH__ */
/*----------------------------------------------------------------------------*/
