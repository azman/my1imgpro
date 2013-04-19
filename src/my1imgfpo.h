/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGFPOH__
#define __MY1IMGFPOH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
struct _iframe
{
	int width,height;
	int length; /* memory is cheap - precalculate this! */
	float *data; /* 1-d vector for 2-d image */
};
typedef struct _iframe my1IFrame;
/*----------------------------------------------------------------------------*/
struct _kernel
{
	int size; /* square mask - should be odd number */
	int length; /* memory is cheap - precalculate this! */
	int orig_x, orig_y; /* origin for kernel placement */
	float *factor;
};
typedef struct _kernel my1Kernel;
/*----------------------------------------------------------------------------*/
/* management functions - image/frame copier/converter */
void initframe(my1IFrame *frame);
void image2frame(my1Image *image, my1IFrame *frame, int align);
void frame2image(my1IFrame *frame, my1Image *image, int align);
/* management functions - decimal (floating-point) framework */
float* createframe(my1IFrame *image, int height, int width);
void freeframe(my1IFrame *image);
void copyframe(my1IFrame *src, my1IFrame *dst);
float framepixel(my1IFrame *image, int row, int col);
void setframepixel(my1IFrame *image, int row, int col,float pixel);
float* createkernel(my1Kernel *kernel, int size);
void freekernel(my1Kernel *kernel);
void setkernel(my1Kernel *kernel, float *farray); /* must set correct size */
/* frame operator */
void getminmax_frame(my1IFrame *frame, float* min, float* max);
void normalize_frame(my1IFrame *frame);
void limitrange_frame(my1IFrame *frame);
void absolute_frame(my1IFrame *frame);
/* other operators */
void correlate_frame(my1IFrame *src, my1IFrame *dst, my1Kernel *kernel);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
