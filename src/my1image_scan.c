/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_SCANC__
#define __MY1IMAGE_SCANC__
/*----------------------------------------------------------------------------*/
#include "my1image_scan.h"
/*----------------------------------------------------------------------------*/
void iscan_init(my1image_scan_t* scan, my1image_t* data, int skip)
{
	scan->data = data->data;
	scan->rows = data->rows;
	scan->cols = data->cols;
	scan->skip = skip;
	scan->brow = scan->rows - skip - 1;
	scan->bcol = scan->cols - skip - 1;
}
/*----------------------------------------------------------------------------*/
void iscan_prep(my1image_scan_t* scan)
{
	scan->curr = scan->data;
	scan->next = scan->curr + scan->cols;
	scan->prev = scan->curr; /* just in case */
	scan->irow = 0;
	scan->icol = -1;
	scan->loop = -1;
}
/*----------------------------------------------------------------------------*/
int iscan_next(my1image_scan_t* scan)
{
	int next = 1;
	scan->loop++;
	scan->icol++;
	if (scan->icol==scan->cols)
	{
		scan->icol = 0;
		scan->irow++;
		scan->prev = scan->curr;
		scan->curr = scan->next;
		scan->next += scan->cols;
		if (scan->irow==scan->rows)
		{
			scan->irow = 0;
			scan->icol = -1;
			scan->loop = -1;
			next = 0;
		}
	}
	return next;
}
/*----------------------------------------------------------------------------*/
int iscan_skip(my1image_scan_t* scan)
{
	return iscan_skip_test(scan,scan->icol,scan->irow);
}
/*----------------------------------------------------------------------------*/
int iscan_skip_loop(my1image_scan_t* scan, int loop)
{
	int trow = loop / scan->cols;
	int tcol = loop % scan->cols;
	return iscan_skip_test(scan,tcol,trow);
}
/*----------------------------------------------------------------------------*/
int iscan_skip_that(my1image_scan_t* scan, int trow, int tcol)
{
	return iscan_skip_test(scan,tcol,trow);
}
/*----------------------------------------------------------------------------*/
int iscan_8connected_base(my1image_scan_t* scan)
{
	int loop, size, curr, step;
	curr = scan->icol;
	step = curr-1;
	for (loop=0,size=0;loop<3;loop++)
	{
		if (scan->prev[step]) size++;
		if (scan->curr[step]) if (step!=curr) size++;
		if (scan->next[step]) size++;
		step++;
	}
	return size;
}
/*----------------------------------------------------------------------------*/
int iscan_8connected(my1image_scan_t* scan)
{
	if (!scan->curr[scan->icol]) return 0;
	return iscan_8connected_base(scan);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_SCANC__ */
/*----------------------------------------------------------------------------*/
