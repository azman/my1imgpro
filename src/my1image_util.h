/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_UTILH__
#define __MY1IMAGE_UTILH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
typedef struct _my1image_region_t
{
	int xset, yset;
	int width, height;
}
my1image_region_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1image_mask_t
{
	int size; /* square mask - should be odd number */
	int length; /* memory is cheap - precalculate this! */
	int origin; /* origin for mask placement - always size/2 */
	int *factor;
}
my1image_mask_t;
/*----------------------------------------------------------------------------*/
typedef my1image_t* (*pfilter_t)(my1image_t* curr, my1image_t* next,
	void* user, void* pass);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_filter_t
{
	void *userdata;
	void *passdata;
	my1image_t buffer;
	pfilter_t filter;
	struct _my1image_filter_t *next; /* linked list */
}
my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1image_histogram_t
{
	int maxindex;
	int count[GRAYLEVEL]; /* grayscale level count */
}
my1image_histogram_t;
/*----------------------------------------------------------------------------*/
/* region@sub-image management functions */
void image_get_region(my1image_t *img, my1image_t *sub,my1image_region_t *reg);
void image_set_region(my1image_t *img, my1image_t *sub,my1image_region_t *reg);
void image_put_region(my1image_t *img, int val, my1image_region_t *reg);
/* mask management functions */
int* image_mask_init(my1image_mask_t *mask, int size);
void image_mask_free(my1image_mask_t *mask);
void image_mask_make(my1image_mask_t *mask, int *pval);
/* linear filter : cross-correlation & convolution */
void image_correlation(my1image_t *dst, my1image_t *src, my1image_mask_t *mask);
void image_convolution(my1image_t *dst, my1image_t *src, my1image_mask_t *mask);
/* generic filter */
void filter_init(my1image_filter_t* pfilter, pfilter_t filter);
void filter_free(my1image_filter_t* pfilter);
my1image_filter_t* filter_insert(my1image_filter_t* pstack,
	my1image_filter_t* pcheck);
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter);
/* grayscale histogram utility */
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist);
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
