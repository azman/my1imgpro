/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_WORKH__
#define __MY1IMAGE_WORKH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_resize(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* image_mask_this(my1image_t* img,my1image_t* res,
	int mask_size, int data_size, int* pdata);
my1image_t* filter_laplace_1(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_laplace_2(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_sobel(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_gauss(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_maxscale(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_suppress(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_threshold(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
my1image_t* filter_canny(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
