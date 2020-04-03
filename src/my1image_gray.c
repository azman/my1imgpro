/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_GRAYC__
#define __MY1IMAGE_GRAYC__
/*----------------------------------------------------------------------------*/
#include "my1image_gray.h"
/*----------------------------------------------------------------------------*/
void image_limit(my1image_t *image)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]>WHITE)
			image->data[loop] = WHITE;
		else if (image->data[loop]<BLACK)
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_invert(my1image_t *image)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
		image->data[loop] = WHITE - image->data[loop];
}
/*----------------------------------------------------------------------------*/
void image_absolute(my1image_t *image)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]<0)
			image->data[loop] = -image->data[loop];
	}
}
/*----------------------------------------------------------------------------*/
void image_binary(my1image_t *image, int threshold, int white)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]>threshold)
			image->data[loop] = white;
		else
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_range(my1image_t *image, int lothresh, int hithresh)
{
	int loop, temp, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		temp = image->data[loop];
		if (temp>hithresh||temp<lothresh) /* black if out of range [lo,hi] */
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliphi(my1image_t *image, int hithresh)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]>hithresh)
			image->data[loop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliplo(my1image_t *image, int lothresh)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]<lothresh)
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_shift(my1image_t *image, int value)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
		image->data[loop] += value;
}
/*----------------------------------------------------------------------------*/
void image_scale(my1image_t *image, float value)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
		image->data[loop] = (int)((float)image->data[loop]*value);
}
/*----------------------------------------------------------------------------*/
void image_normalize(my1image_t *image)
{
	int loop, size = image->size;
	int diff, temp, max = image->data[0], min = image->data[0];
	/* get min max range */
	for (loop=1;loop<size;loop++)
	{
		temp = image->data[loop];
		if (temp>max) max = temp;
		if (temp<min) min = temp;
	}
	/* normalize to min-max scale! */
	diff = max-min;
	for (loop=0;loop<size;loop++)
		image->data[loop] = (image->data[loop]-min)*WHITE/diff;
}
/*----------------------------------------------------------------------------*/
/** should move these to another module? these are filters? */
/*----------------------------------------------------------------------------*/
void image_pan(my1image_t *image, my1image_t *check, int shx, int shy, int vin)
{
	int calx, caly, calv;
	int irow, icol;
	int rows = image->rows, cols = image->cols;
	image_make(check,rows,cols);
	/* calculate new pixel index */
	for (irow=0;irow<rows;irow++)
	{
		for (icol=0;icol<cols;icol++)
		{
			calx = icol - shx;
			caly = irow - shy;
			if (calx>=0&&calx<cols&&caly>=0&&caly<rows)
			{
				calv = image_get_pixel(image,caly,calx);
				image_set_pixel(check,irow,icol,calv);
			}
			else
			{
				image_set_pixel(check,irow,icol,vin);
			}
		}
	}
	check->mask = image->mask;
}
/*----------------------------------------------------------------------------*/
void image_turn(my1image_t *image, my1image_t *check, int turn)
{
	int irow, icol, rows, cols;
	switch (turn)
	{
		case IMAGE_TURN_090:
		case IMAGE_TURN_270:
			rows = image->cols;
			cols = image->rows;
			break;
		case IMAGE_TURN_180:
		case IMAGE_TURN_000:
		default:
			rows = image->rows;
			cols = image->cols;
			break;
	}
	image_make(check,rows,cols);
	for (irow=0;irow<rows;irow++)
	{
		for (icol=0;icol<cols;icol++)
		{
			int trow = irow, tcol = icol;
			switch (turn)
			{
				case IMAGE_TURN_090:
					trow = icol;
					tcol = rows-irow-1;
					break;
				case IMAGE_TURN_270:
					trow = cols-icol-1;
					tcol = irow;
					break;
				case IMAGE_TURN_180:
					trow = rows-irow-1;
					tcol = cols-icol-1;
					break;
			}
			image_set_pixel(check,irow,icol,
				image_get_pixel(image,trow,tcol));
		}
	}
	check->mask = image->mask;
}
/*----------------------------------------------------------------------------*/
void image_flip(my1image_t *image, my1image_t *check, int side)
{
	int irow, icol;
	int rows = image->rows, cols = image->cols;
	image_make(check,rows,cols);
	for (irow=0;irow<rows;irow++)
	{
		for (icol=0;icol<cols;icol++)
		{
			int trow = irow, tcol = icol;
			switch (side)
			{
				case IMAGE_FLIP_HORIZONTAL:
					trow = irow;
					tcol = cols-icol-1;
					break;
				case IMAGE_FLIP_VERTICAL:
					trow = rows-irow-1;
					tcol = icol;
					break;
			}
			image_set_pixel(check,irow,icol,
				image_get_pixel(image,trow,tcol));
		}
	}
	check->mask = image->mask;
}
/*----------------------------------------------------------------------------*/
void image_size_down(my1image_t* image, my1image_t* check,
	int height, int width)
{
	int rows, cols, loop = 0;
	int irow, icol, temp;
	my1image_t count;
	image_init(&count);
	image_make(&count,height,width);
	image_fill(&count,0);
	/* prepare output image */
	image_make(check,height,width);
	image_fill(check,0);
	/* browse all rows and cols in image */
	for(rows=0;rows<image->rows;rows++)
	{
		for(cols=0;cols<image->cols;cols++,loop++)
		{
			/* check where this pixel fits in the smaller image */
			irow = rows*check->rows/image->rows;
			icol = cols*check->cols/image->cols;
			/* validate position */
			if (irow>=check->rows||icol>=check->cols)
				continue;
			/* get index */
			temp = irow*check->cols+icol;
			check->data[temp] += image->data[loop];
			count.data[temp]++;
		}
	}
	check->mask = image->mask;
	/* update all rows and cols in check */
	for(loop=0;loop<check->size;loop++)
	{
		if (!count.data[loop]) continue;
		/* get average */
		check->data[loop] /= count.data[loop];
	}
	image_free(&count);
}
/*----------------------------------------------------------------------------*/
void image_size_up(my1image_t* image, my1image_t* check,
	int height, int width)
{
	int rows, cols, loop = 0;
	int irow, icol, temp;
	/* simple assignment, no fancy interpolation... for now */
	/* prepare output image */
	image_make(check,height,width);
	/* browse all rows and cols in check */
	for(rows=0;rows<check->rows;rows++)
	{
		for(cols=0;cols<check->cols;cols++,loop++)
		{
			/* find pixels in smaller image */
			irow = rows*image->rows/check->rows;
			icol = cols*image->cols/check->cols;
			/* validate position */
			if (irow>=image->rows||icol>=image->cols)
				continue;
			/* get index */
			temp = irow*image->cols+icol;
			/* simply assign */
			check->data[loop] = image->data[temp];
		}
	}
	check->mask = image->mask;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_GRAYC__ */
/*----------------------------------------------------------------------------*/
