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
typedef void (*pfsetup_t)(struct _my1image_filter_t* filter);
typedef void (*pfclean_t)(struct _my1image_filter_t* filter);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_filter_t
{
	char name[FILTER_NAMESIZE];
	void *data; /* pointer to user-defined data */
	my1image_buffer_t *buffer; /* external shared buffer */
	my1image_t *output; /* output image to write to */
	pfilter_t filter; /* pointer to filter function */
	pfsetup_t doinit; /* pointer to setup function */
	pfclean_t dofree; /* pointer to cleanup function */
	struct _my1image_filter_t *next, *last; /* linked list - last in list */
}
my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef struct _filter_info_t
{
	char name[FILTER_NAMESIZE];
	pfilter_t filter;
	pfsetup_t fsetup;
	pfclean_t fclean;
}
filter_info_t;
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
void image_get_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
void image_set_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
void image_area_init(my1image_area_t *reg);
void image_area_make(my1image_area_t *reg, int y, int x, int h, int w);
void image_area_select(my1image_t *img, my1image_area_t *reg, int val, int inv);
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
/* SHOULD NOT be used if dynamic filters use next to create link list */
void filter_unlink(my1image_filter_t* pfilter);
my1image_filter_t* filter_insert(my1image_filter_t* pstack,
	my1image_filter_t* pcheck);
my1image_filter_t* filter_search(my1image_filter_t* ppass, char *name);
/* allow filters to be on heap (dynamic allocation) */
my1image_filter_t* filter_clone(my1image_filter_t* ppass);
void filter_clean(my1image_filter_t* ppass); /* destroys all instances */
/* malloc'ed filter structure */
my1image_filter_t* info_create_filter(filter_info_t* info);
/* apply filter on image */
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter);
/* grayscale histogram utility */
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist);
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist);
/* histogram threshold utility */
void histogram_get_threshold(my1image_histogram_t *hist);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
