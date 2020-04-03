/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_DATAH__
#define __MY1IMAGE_DATAH__
/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
#include "my1image_appw.h"
#include "my1image_hist.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_MAX_WIDTH 640
#define DEFAULT_MAX_HEIGHT 480
/*----------------------------------------------------------------------------*/
#define STATUS_TIMEOUT 5
/*----------------------------------------------------------------------------*/
#define DATA_FLAG_OK 0
#define DATA_FLAG_ERROR 1
/*----------------------------------------------------------------------------*/
typedef struct _my1image_data_t
{
	int dosize, maxh, maxw, flag;
	my1image_t *image;
	my1image_appw_t appw;
	my1image_hist_t hist;
	my1image_buffer_t work;
	my1image_filter_t *pflist, *pfcurr;
}
my1image_data_t;
/*----------------------------------------------------------------------------*/
void image_data_init(my1image_data_t* data);
void image_data_free(my1image_data_t* data);
void image_data_draw(my1image_data_t* data);
void image_data_make(my1image_data_t* data, my1image_t* that);
void image_data_work(my1image_data_t* data);
void image_data_events(my1image_data_t* data);
void image_data_domenu(my1image_data_t* data);
void image_data_filter_more(my1image_data_t* data, filter_info_t* info);
void image_data_filter_load(my1image_data_t* data, char* name);
void image_data_filter_exec(my1image_data_t* data);
/*----------------------------------------------------------------------------*/
void image_data_histogram(void* data);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_DATAH__ */
/*----------------------------------------------------------------------------*/
