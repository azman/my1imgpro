/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_CRGBH__
#define __MY1IMAGE_CRGBH__
/*----------------------------------------------------------------------------*/
#include "my1image_argb.h"
/*----------------------------------------------------------------------------*/
/* rgb conversion utility */
int encode_bgr(cbyte r, cbyte g, cbyte b); /* special: alpha channel is 0xff */
/* swap b and r channels */
int color_swap(int data);
/* color information - structure member 'mask' MUST BE assigned! */
/** will be changed to image_from_rgb */
int image_make_rgb(my1image_t *image, cbyte *rgb);
/** was image_from_rgb */
int image_form_rgb(my1image_t *image, cbyte *rgb);
/* enforce color modes */
void image_grayscale(my1image_t *image);
void image_colormode(my1image_t *image);
/* color-aware invert */
void image_invert_this(my1image_t *image);
/* hack for gdk_draw_rgb_32_image - requires alpha to be 0xff */
void image_copy_color2bgr(my1image_t *dst, my1image_t *src);
/* to recover from above! */
void image_copy_color2rgb(my1image_t *dst, my1image_t *src);
/* color channel extract */
void image_copy_color_channel(my1image_t *dst, my1image_t *src, int mask);
/* color aware image sizing */
my1image_t* image_size_this(my1image_t* image, my1image_t* check,
	int height, int width);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_CRGBH__ */
/*----------------------------------------------------------------------------*/
