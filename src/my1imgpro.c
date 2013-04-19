/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
#include <stdlib.h> /* for malloc and free? */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void initimage(my1Image *image)
{
	image->width = 0;
	image->height = 0;
	image->length = 0;
	image->mask = 0;
	image->data = 0x0;
}
/*----------------------------------------------------------------------------*/
int* createimage(my1Image *image, int height, int width)
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
void freeimage(my1Image *image)
{
	if(image->length) free(image->data);
	image->data = 0x0;
	image->length = 0;
}
/*----------------------------------------------------------------------------*/
void copyimage(my1Image *src, my1Image *dst)
{
	int iloop;
	for(iloop=0;iloop<dst->length;iloop++)
	{
		dst->data[iloop] = src->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void fillimage(my1Image *image, int value)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		image->data[iloop] = value;
	}
}
/*----------------------------------------------------------------------------*/
int imagepixel(my1Image *image, int row, int col) /* col(x),row(y) */
{
	return image->data[row*image->width+col];
}
/*----------------------------------------------------------------------------*/
void setimagepixel(my1Image *image, int row, int col, int pixel)
{
	image->data[row*image->width+col] = pixel;
}
/*----------------------------------------------------------------------------*/
int* imgrowdata(my1Image *image, int row)
{
	return &(image->data[row*image->width]);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void limit_pixel(my1Image *image)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]>WHITE) image->data[iloop] = WHITE;
		else if(image->data[iloop]<BLACK) image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void negate_pixel(my1Image *image)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		image->data[iloop] = WHITE - image->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void absolute_pixel(my1Image *image)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]<0)
			image->data[iloop] = -image->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void binary_pixel(my1Image *image, int threshold)
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
void range_pixel(my1Image *image, int lo, int hi)
{
	int temp, iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		temp = image->data[iloop];
		if(temp>hi||temp<lo) /* check if out of range [lo,hi] */
			image->data[iloop] = BLACK;
		else /* hilite in-range pixels */
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void cliphi_pixel(my1Image *image, int hithresh)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]>hithresh)
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void cliplo_pixel(my1Image *image, int lothresh)
{
	int iloop, ilength = image->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		if(image->data[iloop]<lothresh)
			image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void shift_pixel(my1Image *image, int value)
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
void scale_pixel(my1Image *image, float value)
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
/*----------------------------------------------------------------------------*/
void image_add(my1Image *image1, my1Image *image2, my1Image *result)
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
void image_sub(my1Image *image1, my1Image *image2, my1Image *result)
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
void image_pan(my1Image *image, my1Image *result, int shx, int shy, int vin)
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
				calcv = imagepixel(image,calcy,calcx);
				setimagepixel(result,iloop,jloop,calcv);
			}
			else
			{
				setimagepixel(result,iloop,jloop,vin);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
