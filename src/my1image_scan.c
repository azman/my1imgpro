/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_SCANC__
#define __MY1IMAGE_SCANC__
/*----------------------------------------------------------------------------*/
#include "my1image_scan.h"
/*----------------------------------------------------------------------------*/
void iscan_init(my1image_scan_t* scan, int* data, int rows, int cols, int skip)
{
	scan->data = data;
	scan->rows = rows;
	scan->cols = cols;
	scan->skip = skip;
	scan->brow = rows - skip - 1;
	scan->bcol = cols - skip - 1;
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
	if (scan->irow<scan->skip||scan->irow>scan->brow||
			scan->icol<scan->skip||scan->icol>scan->bcol)
		return 1;
	else return 0;
}
/*----------------------------------------------------------------------------*/
int iscan_skip_loop(my1image_scan_t* scan, int loop)
{
	int trow = loop / scan->cols;
	int tcol = loop % scan->cols;
	if (trow<scan->skip||trow>scan->brow||tcol<scan->skip||tcol>scan->bcol)
		return 1;
	else return 0;
}
/*----------------------------------------------------------------------------*/
int iscan_skip_that(my1image_scan_t* scan, int trow, int tcol)
{
	if (trow<scan->skip||trow>scan->brow||tcol<scan->skip||tcol>scan->bcol)
		return 1;
	else return 0;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_SCANC__ */
/*----------------------------------------------------------------------------*/
