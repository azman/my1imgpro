/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_GRAYH__
#define __MY1IMAGE_GRAYH__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
/* 8-bit graylevel */
#define GRAYLEVEL 256
#define WHITE 255
#define BLACK 0
/*----------------------------------------------------------------------------*/
void image_limit(my1image_t *image);
void image_invert(my1image_t *image);
void image_absolute(my1image_t *image);
void image_binary(my1image_t *image, int threshold, int white);
void image_range(my1image_t *image, int lothresh, int hithresh);
void image_cliphi(my1image_t *image, int hithresh);
void image_cliplo(my1image_t *image, int lothresh);
void image_shift(my1image_t *image, int value);
void image_scale(my1image_t *image, float value);
void image_normalize(my1image_t *image); /* get max-min value, rescale */
/*----------------------------------------------------------------------------*/
/* image functions - pan, rotate, flip, resize */
void image_pan(my1image_t *image, my1image_t *check,
	int shx, int shy, int vin); /* pan-x,pan-y,value-in */
/*----------------------------------------------------------------------------*/
#define IMAGE_TURN_000 0
#define IMAGE_TURN_090 1
#define IMAGE_TURN_180 2
#define IMAGE_TURN_270 3
/*----------------------------------------------------------------------------*/
#define IMAGE_FLIP_VERTICAL 0
#define IMAGE_FLIP_HORIZONTAL 1
#define IMAGE_FLIP_V IMAGE_FLIP_VERTICAL
#define IMAGE_FLIP_H IMAGE_FLIP_HORIZONTAL
/*----------------------------------------------------------------------------*/
void image_turn(my1image_t *image, my1image_t *check, int turn); /* turn*90 */
void image_flip(my1image_t *image, my1image_t *check, int side); /* h/v */
/*----------------------------------------------------------------------------*/
void image_size_down(my1image_t* image, my1image_t* check,
	int height, int width);
void image_size_up(my1image_t* image, my1image_t* check,
	int height, int width);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_GRAYH__ */
/*----------------------------------------------------------------------------*/
