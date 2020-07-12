/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_SCANH__
#define __MY1IMAGE_SCANH__
/*----------------------------------------------------------------------------*/
/**
 *  my1image_scan_t
 *  - special structure for raster scanning image
 *  - keep pointers for previous and next rows
 *  - can specify border pixels to skip
**/
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
#define MY1IMAGE_SCAN_SKIP 1
/*----------------------------------------------------------------------------*/
typedef struct _my1image_scan_t
{
	int *data, *curr, *prev, *next;
	int cols, rows;
	int icol, irow, loop;
	int bcol, brow, skip; /* border mark */
}
my1image_scan_t;
/*----------------------------------------------------------------------------*/
void iscan_init(my1image_scan_t* scan, my1image_t* data, int skip);
void iscan_prep(my1image_scan_t* scan);
int iscan_next(my1image_scan_t* scan);
int iscan_skip(my1image_scan_t* scan);
int iscan_skip_loop(my1image_scan_t* scan, int loop);
int iscan_skip_that(my1image_scan_t* scan, int trow, int tcol);
int iscan_8connected(my1image_scan_t* scan); /* assumes skip > 0! */
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_SCANH__ */
/*----------------------------------------------------------------------------*/
