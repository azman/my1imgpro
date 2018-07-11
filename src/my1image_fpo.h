/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FPOH__
#define __MY1IMAGE_FPOH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
typedef struct _my1frame_t
{
	int width,height;
	int length; /* memory is cheap - precalculate this! */
	float *data; /* 1-d vector for 2-d image */
}
my1frame_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1frame_kernel_t
{
	int size; /* square mask - should be odd number */
	int length; /* memory is cheap - precalculate this! */
	int origin; /* origin for kernel placement - always size/2 */
	float *factor;
}
my1frame_kernel_t;
/*----------------------------------------------------------------------------*/
typedef my1frame_kernel_t my1kernel_t;
/*----------------------------------------------------------------------------*/
/* management functions - decimal (floating-point) framework */
void frame_init(my1frame_t *frame);
void frame_free(my1frame_t *frame);
float* frame_make(my1frame_t *frame, int height, int width);
void frame_copy(my1frame_t *dst, my1frame_t *src);
float frame_get_pixel(my1frame_t *frame, int row, int col);
void frame_set_pixel(my1frame_t *frame, int row, int col, float pixel);
float* frame_row_data(my1frame_t *frame, int row);
/* management functions - image/frame copier/converter */
void frame_get_image(my1frame_t *frame, my1image_t *image, int align);
void frame_set_image(my1frame_t *frame, my1image_t *image, int align);
/* frame mask kernel */
float* frame_kernel_init(my1frame_kernel_t *kernel, int size);
void frame_kernel_free(my1frame_kernel_t *kernel);
void frame_kernel_make(my1frame_kernel_t *kernel, int size, float *farray);
/* frame operator */
void frame_get_minmax(my1frame_t *frame, float* min, float* max);
void frame_normalize(my1frame_t *frame);
void frame_limitrange(my1frame_t *frame);
void frame_absolute(my1frame_t *frame);
/* other operators */
void frame_correlate(my1frame_t *frame, my1frame_t *res, my1kernel_t *kernel);
void frame_convolute(my1frame_t *frame, my1frame_t *res, my1kernel_t *kernel);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
