/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /** for malloc and free? */
/*----------------------------------------------------------------------------*/
void image_init(my1image_t *image)
{
	image->width = 0;
	image->height = 0;
	image->length = 0;
	image->mask = 0;
	image->data = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_free(my1image_t *image)
{
	if (image->data) free((void*)image->data);
	image->data = 0x0;
	image->length = 0;
}
/*----------------------------------------------------------------------------*/
int* image_make(my1image_t *image, int height, int width)
{
	int length = height*width;
	int *temp = image->data;
	if (!temp||image->height!=height||image->width!=width)
	{
		temp = (int*) realloc(image->data,length*sizeof(int));
		if (temp)
		{
			image->data = temp;
			image->height = height;
			image->width = width;
			image->length = length;
		}
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void image_copy(my1image_t *dst, my1image_t *src)
{
	int loop;
	image_make(dst,src->height,src->width);
	for (loop=0;loop<dst->length;loop++)
		dst->data[loop] = src->data[loop];
	dst->mask = src->mask;
}
/*----------------------------------------------------------------------------*/
void image_fill(my1image_t *image, int value)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
		image->data[loop] = value;
}
/*----------------------------------------------------------------------------*/
int image_get_pixel(my1image_t *image, int row, int col) /* col(x),row(y) */
{
	return image->data[row*image->width+col];
}
/*----------------------------------------------------------------------------*/
void image_set_pixel(my1image_t *image, int row, int col, int pixel)
{
	image->data[row*image->width+col] = pixel;
}
/*----------------------------------------------------------------------------*/
int* image_row_data(my1image_t *image, int row)
{
	return &(image->data[row*image->width]);
}
/*----------------------------------------------------------------------------*/
void image_limit(my1image_t *image)
{
	int loop, size = image->length;
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
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
		image->data[loop] = WHITE - image->data[loop];
}
/*----------------------------------------------------------------------------*/
void image_absolute(my1image_t *image)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]<0)
			image->data[loop] = -image->data[loop];
	}
}
/*----------------------------------------------------------------------------*/
void image_binary(my1image_t *image, int threshold)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]>threshold)
			image->data[loop] = WHITE;
		else
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_range(my1image_t *image, int lothresh, int hithresh)
{
	int loop, size = image->length, temp;
	for (loop=0;loop<size;loop++)
	{
		temp = image->data[loop];
		if (temp>hithresh||temp<lothresh) /* black if out of range [lo,hi] */
			image->data[loop] = BLACK;
		else /* hilite in-range pixels */
			image->data[loop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliphi(my1image_t *image, int hithresh)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]>hithresh)
			image->data[loop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliplo(my1image_t *image, int lothresh)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		if (image->data[loop]<lothresh)
			image->data[loop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_shift(my1image_t *image, int value)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		int temp = image->data[loop] + value;
		if (temp>WHITE) temp = WHITE;
		else if (temp<BLACK) temp = BLACK;
		image->data[loop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_scale(my1image_t *image, float value)
{
	int loop, size = image->length;
	for (loop=0;loop<size;loop++)
	{
		float temp = (float) image->data[loop]*value;
		if (temp>(float)WHITE) temp = (float) WHITE;
		image->data[loop] = (int) temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_normalize(my1image_t *image)
{
	int loop, size = image->length;
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
void image_pan(my1image_t *image, my1image_t *check, int shx, int shy, int vin)
{
	int calx, caly, calv;
	int irow, icol;
	int rows = image->height, cols = image->width;
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
			rows = image->width;
			cols = image->height;
			break;
		case IMAGE_TURN_180:
		case IMAGE_TURN_000:
		default:
			rows = image->height;
			cols = image->width;
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
	int rows = image->height, cols = image->width;
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
	for(rows=0;rows<image->height;rows++)
	{
		for(cols=0;cols<image->width;cols++,loop++)
		{
			/* check where this pixel fits in the smaller image */
			irow = rows*check->height/image->height;
			icol = cols*check->width/image->width;
			/* validate position */
			if (irow>=check->height||icol>=check->width)
				continue;
			/* get index */
			temp = irow*check->width+icol;
			check->data[temp] += image->data[loop];
			count.data[temp]++;
		}
	}
	check->mask = image->mask;
	/* update all rows and cols in check */
	for(loop=0;loop<check->length;loop++)
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
	for(rows=0;rows<check->height;rows++)
	{
		for(cols=0;cols<check->width;cols++,loop++)
		{
			/* find pixels in smaller image */
			irow = rows*image->height/check->height;
			icol = cols*image->width/check->width;
			/* validate position */
			if (irow>=image->height||icol>=image->width)
				continue;
			/* get index */
			temp = irow*image->width+icol;
			/* simply assign */
			check->data[loop] = image->data[temp];
		}
	}
	check->mask = image->mask;
}
/*----------------------------------------------------------------------------*/
