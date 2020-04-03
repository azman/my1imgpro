/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_SCANH__
#define __MY1IMAGE_SCANH__
/*----------------------------------------------------------------------------*/
/**
 *  my1image_scan_t
 *  - special structure for raster scanning image
 *  - keep pointers for previous and next rows
 *  - only this header is needed :p
**/
/*----------------------------------------------------------------------------*/
#define MY1IMAGE_SCAN_SKIP 1
/*----------------------------------------------------------------------------*/
typedef struct _my1image_scan_t
{
	int cols, rows, loop, *data;
	int icol, irow, *curr, *prev, *next;
	int bcol, brow, skip; /* border mark */
}
my1image_scan_t;
/*----------------------------------------------------------------------------*/
void scan_init(my1image_scan_t* scan, int* data, int rows, int cols, int skip);
void scan_prep(my1image_scan_t* scan);
int scan_next(my1image_scan_t* scan);
int scan_skip(my1image_scan_t* scan);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_SCANH__ */
/*----------------------------------------------------------------------------*/
