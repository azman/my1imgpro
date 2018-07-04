/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
#include <stdlib.h> /* for malloc and free? */
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
int* image_make(my1image_t *image, int height, int width)
{
	int length = height*width;
	int *temp = (int*) malloc(length*sizeof(int));
	if(temp)
	{
		image->data = temp;
		image->height = height;
		image->width = width;
		image->length = length;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void image_free(my1image_t *image)
{
	if(image->data) free((void*)image->data);
	image->data = 0x0;
	image->length = 0;
}
/*----------------------------------------------------------------------------*/
void image_copy(my1image_t *src, my1image_t *dst)
{
	int iloop;
	for(iloop=0;iloop<dst->length;iloop++)
		dst->data[iloop] = src->data[iloop];
}
/*----------------------------------------------------------------------------*/
void image_fill(my1image_t *image, int value)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
		image->data[iloop] = value;
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
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]>WHITE) image->data[iloop] = WHITE;
		else if(image->data[iloop]<BLACK) image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_invert(my1image_t *image)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
		image->data[iloop] = WHITE - image->data[iloop];
}
/*----------------------------------------------------------------------------*/
void image_absolute(my1image_t *image)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]<0)
			image->data[iloop] = -image->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void image_threshold(my1image_t *image, int threshold)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]>threshold)
			image->data[iloop] = WHITE;
		else
			image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_range(my1image_t *image, int lothresh, int hithresh)
{
	int temp, iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		temp = image->data[iloop];
		if(temp>hithresh||temp<lothresh) /* check if out of range [lo,hi] */
			image->data[iloop] = BLACK;
		else /* hilite in-range pixels */
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliphi(my1image_t *image, int hithresh)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]>hithresh)
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliplo(my1image_t *image, int lothresh)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]<lothresh)
			image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_shift(my1image_t *image, int value)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		int temp = image->data[iloop] + value;
		if(temp>WHITE) temp = WHITE;
		else if(temp<BLACK) temp = BLACK;
		image->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_scale(my1image_t *image, float value)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		float temp = (float) image->data[iloop]*value;
		if(temp>(float)WHITE) temp = (float) WHITE;
		image->data[iloop] = (int) temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_scale_range(my1image_t *image)
{
	float scale;
	int iloop, ilength = image->length;
	int max = image->data[0], min = image->data[0];
	/* get min max range */
	for(iloop=1;iloop<ilength;iloop++)
	{
		if(max<image->data[iloop]) max = image->data[iloop];
		else if(min>image->data[iloop]) min = image->data[iloop];
	}
	/* normalize to min-max scale! */
	scale = (float)(max-min)/WHITE;
	for(iloop=0;iloop<ilength;iloop++)
	{
		image->data[iloop] = (int) ((image->data[iloop]-min)/scale);
	}
}
/*----------------------------------------------------------------------------*/
void image_add(my1image_t *image1, my1image_t *image2, my1image_t *result)
{
	int iloop, ilength = image1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		int temp = image1->data[iloop] + image2->data[iloop];
		if(temp>WHITE) temp = WHITE;
		else if(temp<BLACK) temp = BLACK;
		result->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_sub(my1image_t *image1, my1image_t *image2, my1image_t *result)
{
	int iloop, ilength = image1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		int temp = image1->data[iloop] - image2->data[iloop];
		if(temp>WHITE) temp = WHITE;
		else if(temp<BLACK) temp = BLACK;
		result->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_pan(my1image_t *image, my1image_t *result, int shx, int shy, int vin)
{
	int iloop, jloop;
	int calcx, calcy, calcv;
	int row = image->height, col = image->width;
	/* calculate new pixel index */
	for(iloop=0;iloop<row;iloop++)
	{
		for(jloop=0;jloop<col;jloop++)
		{
			calcx = jloop - shx;
			calcy = iloop - shy;
			if(calcx>=0&&calcx<col&&calcy>=0&&calcy<row)
			{
				calcv = image_get_pixel(image,calcy,calcx);
				image_set_pixel(result,iloop,jloop,calcv);
			}
			else
			{
				image_set_pixel(result,iloop,jloop,vin);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
