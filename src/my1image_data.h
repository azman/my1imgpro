/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_DATAH__
#define __MY1IMAGE_DATAH__
/*----------------------------------------------------------------------------*/
#include "my1image_hist.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_MAX_WIDTH 640
#define DEFAULT_MAX_HEIGHT 480
/*----------------------------------------------------------------------------*/
typedef struct _my1image_data_t
{
	int dosize, maxh, maxw;
	my1image_hist_t hist;
	my1image_t buff;
	char *list; /* filter list from args */
}
my1image_data_t;
/*----------------------------------------------------------------------------*/
int image_data_histogram(void* data, void* that, void* xtra);
/*----------------------------------------------------------------------------*/
int image_data_init(void* data, void* that, void* xtra);
int image_data_free(void* data, void* that, void* xtra);
int image_data_args(void* data, void* that, void* xtra);
int image_data_prep(void* data, void* that, void* xtra);
int image_data_exec(void* data, void* that, void* xtra);
int image_data_show(void* data, void* that, void* xtra);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_DATAH__ */
/*----------------------------------------------------------------------------*/
