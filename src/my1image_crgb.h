/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_CRGBH__
#define __MY1IMAGE_CRGBH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
/* image color (rgb) stuffs */
/*----------------------------------------------------------------------------*/
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
int encode_rgb(cbyte r, cbyte g, cbyte b);
int encode_bgr(cbyte r, cbyte g, cbyte b); /* special: alpha channel is 0xff */
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b);
/* encoded rgb conversion */
int gray2color(int data);
int color2gray(int data);
/* swap b and r channels */
int color_swap(int data);
/* color information - structure member 'mask' MUST BE assigned! */
int image_make_rgb(my1image_t *image, cbyte *rgb);
int image_from_rgb(my1image_t *image, cbyte *rgb);
/* enforce color modes */
void image_grayscale(my1image_t *image);
void image_colormode(my1image_t *image);
/* hack for gdk_draw_rgb_32_image - requires alpha to be 0xff */
void image_copy_color2bgr(my1image_t *dst, my1image_t *src);
/* color channel extract */
void image_copy_color_channel(my1image_t *dst, my1image_t *src, int mask);
/* image sizing - requires color info! */
my1image_t* image_size_this(my1image_t* image, my1image_t* check,
	int height, int width);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
