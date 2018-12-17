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
	int iloop;
	for (iloop=0;iloop<dst->length;iloop++)
		dst->data[iloop] = src->data[iloop];
	dst->mask = src->mask;
}
/*----------------------------------------------------------------------------*/
void image_fill(my1image_t *image, int value)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
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
	for (iloop=0;iloop<ilength;iloop++)
	{
		if (image->data[iloop]>WHITE) image->data[iloop] = WHITE;
		else if (image->data[iloop]<BLACK) image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_invert(my1image_t *image)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
		image->data[iloop] = WHITE - image->data[iloop];
}
/*----------------------------------------------------------------------------*/
void image_absolute(my1image_t *image)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		if (image->data[iloop]<0)
			image->data[iloop] = -image->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void image_binary(my1image_t *image, int threshold)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		if (image->data[iloop]>threshold)
			image->data[iloop] = WHITE;
		else
			image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_range(my1image_t *image, int lothresh, int hithresh)
{
	int temp, iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		temp = image->data[iloop];
		if (temp>hithresh||temp<lothresh) /* check if out of range [lo,hi] */
			image->data[iloop] = BLACK;
		else /* hilite in-range pixels */
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliphi(my1image_t *image, int hithresh)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		if (image->data[iloop]>hithresh)
			image->data[iloop] = WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void image_cliplo(my1image_t *image, int lothresh)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		if (image->data[iloop]<lothresh)
			image->data[iloop] = BLACK;
	}
}
/*----------------------------------------------------------------------------*/
void image_shift(my1image_t *image, int value)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		int temp = image->data[iloop] + value;
		if (temp>WHITE) temp = WHITE;
		else if (temp<BLACK) temp = BLACK;
		image->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_scale(my1image_t *image, float value)
{
	int iloop, ilength = image->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		float temp = (float) image->data[iloop]*value;
		if (temp>(float)WHITE) temp = (float) WHITE;
		image->data[iloop] = (int) temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_normalize(my1image_t *image)
{
	int iloop, ilength = image->length;
	int diff, max = image->data[0], min = image->data[0];
	/* get min max range */
	for (iloop=1;iloop<ilength;iloop++)
	{
		if (max<image->data[iloop]) max = image->data[iloop];
		else if (min>image->data[iloop]) min = image->data[iloop];
	}
	/* normalize to min-max scale! */
	diff = max-min;
	for (iloop=0;iloop<ilength;iloop++)
		image->data[iloop] = (image->data[iloop]-min)*WHITE/diff;
}
/*----------------------------------------------------------------------------*/
void image_add(my1image_t *image1, my1image_t *image2, my1image_t *result)
{
	int iloop, ilength = image1->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		int temp = image1->data[iloop] + image2->data[iloop];
		if (temp>WHITE) temp = WHITE;
		else if (temp<BLACK) temp = BLACK;
		result->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_sub(my1image_t *image1, my1image_t *image2, my1image_t *result)
{
	int iloop, ilength = image1->length;
	for (iloop=0;iloop<ilength;iloop++)
	{
		int temp = image1->data[iloop] - image2->data[iloop];
		if (temp>WHITE) temp = WHITE;
		else if (temp<BLACK) temp = BLACK;
		result->data[iloop] = temp;
	}
}
/*----------------------------------------------------------------------------*/
void image_pan(my1image_t *image, my1image_t *check, int shx, int shy, int vin)
{
	int iloop, jloop;
	int calcx, calcy, calcv;
	int row = image->height, col = image->width;
	/* calculate new pixel index */
	for (iloop=0;iloop<row;iloop++)
	{
		for (jloop=0;jloop<col;jloop++)
		{
			calcx = jloop - shx;
			calcy = iloop - shy;
			if (calcx>=0&&calcx<col&&calcy>=0&&calcy<row)
			{
				calcv = image_get_pixel(image,calcy,calcx);
				image_set_pixel(check,iloop,jloop,calcv);
			}
			else
			{
				image_set_pixel(check,iloop,jloop,vin);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_turn(my1image_t *image, my1image_t *check, int turn)
{
	int irow, icol;
	switch (turn)
	{
		case IMAGE_TURN_090:
		case IMAGE_TURN_270:
			image_make(check,image->width,image->height);
			break;
		case IMAGE_TURN_180:
		case IMAGE_TURN_000:
		default:
			image_make(check,image->height,image->width);
			break;
	}
	for (irow=0;irow<check->height;irow++)
	{
		for (icol=0;icol<check->width;icol++)
		{
			int trow = irow, tcol = icol;
			switch (turn)
			{
				case IMAGE_TURN_090:
					trow = icol;
					tcol = check->height-irow-1;
					break;
				case IMAGE_TURN_270:
					trow = check->width-icol-1;
					tcol = irow;
					break;
				case IMAGE_TURN_180:
					trow = check->height-irow-1;
					tcol = check->width-icol-1;
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
	image_make(check,image->height,image->width);
	for (irow=0;irow<check->height;irow++)
	{
		for (icol=0;icol<check->width;icol++)
		{
			int trow = irow, tcol = icol;
			switch (side)
			{
				case IMAGE_FLIP_HORIZONTAL:
					trow = irow;
					tcol = check->width-icol-1;
					break;
				case IMAGE_FLIP_VERTICAL:
					trow = check->height-irow-1;
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
int image_assign_rgb(my1image_t *image, cbyte *rgb)
{
	char chkr, chkg, chkb;
	int loop, index = 0;
	if (image->mask==IMASK_COLOR24)
	{
		for (loop=0;loop<image->length;loop++)
		{
			image->data[loop] = encode_rgb(rgb[index+2],
				rgb[index+1],rgb[index+0]);
			index += 3;
		}
	}
	else
	{
		for (loop=0;loop<image->length;loop++)
		{
			chkr = rgb[index++];
			chkg = rgb[index++];
			chkb = rgb[index++];
			image->data[loop] = ((int)chkr+chkg+chkb)/3;
		}
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
int image_extract_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, index = 0;
	if (image->mask==IMASK_COLOR24)
	{
		for (loop=0;loop<image->length;loop++)
		{
			decode_rgb(image->data[loop],&rgb[index+2],
				&rgb[index+1],&rgb[index+0]);
			index += 3;
		}
	}
	else
	{
		for (loop=0;loop<image->length;loop++)
		{
			rgb[index++] = (image->data[loop]&0xff);
			rgb[index++] = (image->data[loop]&0xff);
			rgb[index++] = (image->data[loop]&0xff);
		}
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
void image_grayscale(my1image_t *image)
{
	int loop;
	if (image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
			image->data[loop] = color2gray(image->data[loop]);
		image->mask = IMASK_GRAY;
	}
}
/*----------------------------------------------------------------------------*/
void image_colormode(my1image_t *image)
{
	int loop;
	if (image->mask!=IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
			image->data[loop] = gray2color(image->data[loop]);
		image->mask = IMASK_COLOR24;
	}
}
/*----------------------------------------------------------------------------*/
int encode_bgr(cbyte r, cbyte g, cbyte b)
{
	return (((int)b&0xff)<<16) | (((int)g&0xff)<<8) | ((int)r&0xff);
}
/*----------------------------------------------------------------------------*/
void image_color2bgr(my1image_t *image)
{
	int loop;
	if (image->mask==IMASK_COLOR24)
	{
		cbyte *that, swap;
		for(loop=0;loop<image->length;loop++)
		{
			that = (cbyte*) &image->data[loop];
			swap = that[0];
			that[0] = that[2];
			that[2] = swap;
		}
	}
	else
	{
		for(loop=0;loop<image->length;loop++)
			image->data[loop] = encode_bgr(image->data[loop],
				image->data[loop],image->data[loop]);
		image->mask = IMASK_COLOR24;
	}
}
/*----------------------------------------------------------------------------*/
int encode_rgb(cbyte r, cbyte g, cbyte b)
{
	return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);
}
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b)
{
	*r = (data&0xff0000)>>16;
	*g = (data&0xff00)>>8;
	*b = (data&0xff);
}
/*----------------------------------------------------------------------------*/
int color2gray(int data)
{
	cbyte r, g, b;
	decode_rgb(data,&r,&g,&b);
	/** consider luminosity? */
	/*0.21 R + 0.71 G + 0.07 B*/
	/** go for speed? */
	return (((int)g<<1)+(int)r+(int)b)>>2; /* average: (g+g+r+b)/4 */
}
/*----------------------------------------------------------------------------*/
int gray2color(int data)
{
	return encode_rgb(data,data,data);
}
/*----------------------------------------------------------------------------*/
