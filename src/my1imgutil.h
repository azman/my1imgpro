/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGUTILH__
#define __MY1IMGUTILH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
struct _region
{
	int xset, yset;
	int width, height;
};
typedef struct _region my1Region;
/*----------------------------------------------------------------------------*/
struct _mask
{
	int size; /* square mask - should be odd number */
	int length; /* memory is cheap - precalculate this! */
	int orig_x, orig_y; /* origin for mask placement */
	int *factor;
};
typedef struct _mask my1Mask;
/*----------------------------------------------------------------------------*/
struct _hist
{
	int maxindex;
	int count[GRAYLEVEL]; /* grayscale level count */
};
typedef struct _hist my1Hist;
/*----------------------------------------------------------------------------*/
/* sub-image management functions */
void image2sub(my1Image *image, my1Image *subimage, my1Region *region);
void sub2image(my1Image *subimage, my1Image *image, my1Region *region);
void fillregion(my1Image *image, int value, my1Region *region);
/* mask management functions */
int* createmask(my1Mask *mask, int size);
void freemask(my1Mask *mask);
void setmask(my1Mask *mask, int *parray); /* user must set correct size */
/* generic filter/convolution */
void filter_image(my1Image *src, my1Image *dst, my1Mask *mask);
/* grayscale histogram utility */
void histogram_image(my1Image *image, my1Hist *hist);
void histogram_smooth(my1Image *image, my1Hist *hist); /* not much use? */
int histogram_maxindex(my1Hist *hist);
/* enforce grayscale image */
void grayscale_image(my1Image *image);
/* rgb conversion utility */
int encode_rgb(char r, char g, char b);
void decode_rgb(int data, char *r, char *g, char *b);
/* color information - structure member 'mask' MUST BE assigned! */
int extract_rgb(my1Image *image, char *rgb);
int assign_rgb(my1Image *image, char *rgb);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
