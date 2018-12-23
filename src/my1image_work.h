/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_WORKH__
#define __MY1IMAGE_WORKH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res, void* data);
my1image_t* image_size_this(my1image_t* img,my1image_t* res,
	int height, int width);
my1image_t* filter_resize(my1image_t* img, my1image_t* res, void* data);
my1image_t* image_mask_this(my1image_t* img,my1image_t* res,
	int mask_size, int data_size, int* pdata);
my1image_t* filter_laplace_1(my1image_t* img, my1image_t* res, void* data);
my1image_t* filter_laplace_2(my1image_t* img, my1image_t* res, void* data);
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res, void* data);
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res, void* data);
my1image_t* filter_sobel(my1image_t* img, my1image_t* res, void* data);
my1image_t* filter_gauss(my1image_t* img, my1image_t* res, void* data);
void image_non_max_suppress(my1image_t *img, my1image_t *res, my1image_t *chk);
my1image_t* filter_canny(my1image_t* img, my1image_t* res, void* data);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
