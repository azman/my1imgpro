/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_WORKH__
#define __MY1IMAGE_WORKH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
my1image_t* frame_laplace(my1image_t* img, my1image_t* res, void* userdata);
my1image_t* image_mask_this(my1image_t* img,my1image_t* res,
	int mask_size, int data_size, int* pdata);
my1image_t* image_laplace(my1image_t* img, my1image_t* res, void* userdata);
my1image_t* image_sobel_x(my1image_t* img, my1image_t* res, void* userdata);
my1image_t* image_sobel_y(my1image_t* img, my1image_t* res, void* userdata);
my1image_t* image_sobel(my1image_t* img, my1image_t* res, void* userdata);
my1image_t* image_gauss(my1image_t* img, my1image_t* res, void* userdata);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
