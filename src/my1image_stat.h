/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_STATH__
#define __MY1IMAGE_STATH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
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
/* grayscale histogram utility */
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist);
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist);
/* histogram threshold utility */
void histogram_get_threshold(my1image_histogram_t *hist);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
