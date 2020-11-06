/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_BUFFH__
#define __MY1IMAGE_BUFFH__
/*----------------------------------------------------------------------------*/
#include "my1image_area.h"
/*----------------------------------------------------------------------------*/
typedef struct _my1image_buffer_t
{
	my1image_area_t region, select; /* AOI and select/restore info */
	my1image_t *curr, *next, *temp, *iref; /* extra 1 pointer for reference! */
	my1image_t main, buff, xtra; /* extra image cache */
}
my1image_buffer_t;
/*----------------------------------------------------------------------------*/
typedef my1image_buffer_t my1ibuff_t;
/*----------------------------------------------------------------------------*/
/* double buffered image for processing */
void buffer_init(my1ibuff_t* ibuff);
void buffer_free(my1ibuff_t* ibuff);
void buffer_size(my1ibuff_t* ibuff, int height, int width);
void buffer_size_all(my1ibuff_t* ibuff, int height, int width);
void buffer_swap(my1ibuff_t* ibuff);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_BUFFH__ */
/*----------------------------------------------------------------------------*/
