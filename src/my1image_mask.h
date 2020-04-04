/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MASKH__
#define __MY1IMAGE_MASKH__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
typedef struct _my1image_mask_t
{
	int orow, ocol, step; /* for scanning: image pixels offsets */
	int sqsize; /* square mask - should be odd number */
	int length; /* memory is cheap - precalculate this! */
	int origin; /* origin for mask placement - always size/2 */
	int *factor;
}
my1image_mask_t;
/*----------------------------------------------------------------------------*/
/* mask management functions */
int* image_mask_init(my1image_mask_t *mask, int size);
void image_mask_free(my1image_mask_t *mask);
void image_mask_make(my1image_mask_t *mask, int size, int *pval);
void image_mask_prep(my1image_mask_t *mask);
int* image_mask_scan(my1image_mask_t *mask);
/* linear filter : cross-correlation & convolution */
void image_correlation(my1image_t *img, my1image_t *res, my1image_mask_t *mask);
void image_convolution(my1image_t *img, my1image_t *res, my1image_mask_t *mask);
/* useful to have */
my1image_t* image_mask_this(my1image_t* img, my1image_t* res,
	int mask_size, int data_size, int* pdata);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_MASKH__ */
/*----------------------------------------------------------------------------*/
