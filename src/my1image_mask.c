/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MASKC__
#define __MY1IMAGE_MASKC__
/*----------------------------------------------------------------------------*/
#include "my1image_mask.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
//#include <string.h>
/*----------------------------------------------------------------------------*/
int* image_mask_init(my1image_mask_t *mask, int size)
{
	int length = size*size;
	int *temp = (int*) malloc(length*sizeof(int));
	if (temp)
	{
		mask->sqsize = size;
		mask->length = length;
		mask->origin = size/2;
		mask->factor = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void image_mask_free(my1image_mask_t *mask)
{
	free((void*)mask->factor);
	mask->factor = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_mask_make(my1image_mask_t *mask, int size, int *pval)
{
	int loop, temp = mask->length;
	if (size<mask->length) temp = size;
	for (loop=0;loop<temp;loop++)
		mask->factor[loop] = pval[loop];
}
/*----------------------------------------------------------------------------*/
void image_mask_prep(my1image_mask_t *mask)
{
	mask->orow = -mask->origin;
	mask->ocol = -mask->origin-1;
	mask->step = 0;
}
/*----------------------------------------------------------------------------*/
int* image_mask_scan(my1image_mask_t *mask)
{
	int *temp;
	temp = &mask->factor[mask->step];
	mask->step++;
	mask->ocol++;
	if (mask->ocol<=mask->origin)
	{
		mask->ocol = -mask->origin;
		mask->orow++;
		if (mask->orow<=mask->origin)
		{
			mask->orow = -mask->origin;
			mask->ocol--;
			mask->step = -1;
			temp = 0x0;
		}
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
int image_get_valid(my1image_t *image, int row, int col)
{
	if (row<0) row = 0;
	else if (row>=image->rows) row = image->rows-1;
	if (col<0) col = 0;
	else if (col>=image->cols) col = image->cols-1;
	return image->data[row*image->cols+col];
}
/*----------------------------------------------------------------------------*/
void image_correlation(my1image_t *img, my1image_t *res, my1image_mask_t *mask)
{
	int irow, icol, data, mrow, mcol, *temp;
	image_mask_prep(mask);
	for (irow=0;irow<img->rows;irow++)
	{
		for (icol=0;icol<img->cols;icol++)
		{
			data = 0;
			while ((temp=image_mask_scan(mask)))
			{
				/* cross-correlation */
				mrow = irow+mask->orow;
				mcol = icol+mask->ocol;
				data += temp[0] * image_get_valid(img,mrow,mcol);
			}
			image_set_pixel(res,irow,icol,data);
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_convolution(my1image_t *img, my1image_t *res, my1image_mask_t *mask)
{
	int irow, icol, data, mrow, mcol, *temp;
	image_mask_prep(mask);
	for (irow=0;irow<img->rows;irow++)
	{
		for (icol=0;icol<img->cols;icol++)
		{
			data = 0;
			while ((temp=image_mask_scan(mask)))
			{
				/* convolution - mirrored mask? */
				mrow = irow-mask->orow;
				mcol = icol-mask->ocol;
				data += temp[0] * image_get_valid(img,mrow,mcol);
			}
			image_set_pixel(res,irow,icol,data);
		}
	}
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_MASKC__ */
/*----------------------------------------------------------------------------*/
