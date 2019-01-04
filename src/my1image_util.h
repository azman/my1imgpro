/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_UTILH__
#define __MY1IMAGE_UTILH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
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
typedef struct _my1image_area_t
{
	int xset, yset;
	int width, height;
}
my1image_area_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1image_buffer_t
{
	/* double buffer image - with region! */
	my1image_area_t region;
	my1image_t main, buff, xtra, *curr, *next, *temp;
}
my1image_buffer_t;
/*----------------------------------------------------------------------------*/
#define FILTER_NAMESIZE 80
/*----------------------------------------------------------------------------*/
struct _my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef my1image_t* (*pfilter_t)(my1image_t* curr, my1image_t* next,
	struct _my1image_filter_t* filter);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_filter_t
{
	char name[FILTER_NAMESIZE];
	void *data; /* pointer to user-defined data */
	void *parent; /* pointer to parent object */
	my1image_buffer_t *buffer; /* external shared buffer */
	my1image_t *docopy; /* if supplied, copy to this struct! */
	pfilter_t filter; /* pointer to filter function */
	struct _my1image_filter_t *next, *last; /* linked list - last in list */
}
my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1image_histogram_t
{
	int threshold; /* suggested threshold value? */
	int maxindex, chkindex; /* find 2 highest peak */
	int maxvalue, chkvalue;
	int minvalue, minindex;
	int count[GRAYLEVEL]; /* grayscale level count */
}
my1image_histogram_t;
/*----------------------------------------------------------------------------*/
/* mask management functions */
int* image_mask_init(my1image_mask_t *mask, int size);
void image_mask_free(my1image_mask_t *mask);
void image_mask_make(my1image_mask_t *mask, int size, int *pval);
/* linear filter : cross-correlation & convolution */
void image_correlation(my1image_t *img, my1image_t *res, my1image_mask_t *mask);
void image_convolution(my1image_t *img, my1image_t *res, my1image_mask_t *mask);
/* region@sub-image management functions */
void image_get_region(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
void image_set_region(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
void image_fill_region(my1image_t *img, my1image_area_t *reg, int val);
void image_mask_region(my1image_t *img, my1image_area_t *reg, int inv);
void image_size_aspect(my1image_t *img, my1image_area_t *reg);
/* double buffered image for processing */
void buffer_init(my1image_buffer_t* ibuff);
void buffer_free(my1image_buffer_t* ibuff);
void buffer_size(my1image_buffer_t* ibuff, int height, int width);
void buffer_size_all(my1image_buffer_t* ibuff, int height, int width);
void buffer_swap(my1image_buffer_t* ibuff);
/* generic filter */
void filter_init(my1image_filter_t* pfilter,
	pfilter_t filter, my1image_buffer_t *buffer);
void filter_free(my1image_filter_t* pfilter);
my1image_filter_t* filter_insert(my1image_filter_t* pstack,
	my1image_filter_t* pcheck);
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter);
/* grayscale histogram utility */
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist);
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist);
/* histogram threshold utility */
void histogram_get_threshold(my1image_histogram_t *hist);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
