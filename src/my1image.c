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
		if (temp>hithresh||temp<lothresh) /* check if out of range [lo,hi] */
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
int gray4rgb(cbyte r, cbyte g, cbyte b)
{
	return (((int)g<<1)+(int)r+(int)b)>>2; /* average: (g+g+r+b)/4 */
}
/*----------------------------------------------------------------------------*/
int image_assign_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, size = image->length;
	if (image->mask==IMASK_COLOR)
	{
		for (loop=0;loop<size;loop++)
		{
			image->data[loop] = encode_rgb(rgb[0],rgb[1],rgb[2]);
			rgb += 3;
		}
	}
	else
	{
		for (loop=0;loop<size;loop++)
		{
			image->data[loop] = gray4rgb(rgb[0],rgb[1],rgb[2]);
			rgb += 3;
		}
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
int image_extract_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, size = image->length;
	if (image->mask==IMASK_COLOR)
	{
		for (loop=0;loop<size;loop++)
		{
			decode_rgb(image->data[loop],&rgb[0],&rgb[1],&rgb[2]);
			rgb += 3;
		}
	}
	else
	{
		for (loop=0;loop<size;loop++)
		{
			*rgb = (image->data[loop]&0xff); rgb++;
			*rgb = (image->data[loop]&0xff); rgb++;
			*rgb = (image->data[loop]&0xff); rgb++;
		}
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
void image_grayscale(my1image_t *image)
{
	if (image->mask==IMASK_COLOR)
	{
		int loop, size = image->length;
		for (loop=0;loop<size;loop++)
			image->data[loop] = color2gray(image->data[loop]);
		image->mask = IMASK_GRAY;
	}
}
/*----------------------------------------------------------------------------*/
void image_colormode(my1image_t *image)
{
	if (image->mask!=IMASK_COLOR)
	{
		int loop, size = image->length;
		for (loop=0;loop<size;loop++)
			image->data[loop] = gray2color(image->data[loop]);
		image->mask = IMASK_COLOR;
	}
}
/*----------------------------------------------------------------------------*/
int encode_bgr(cbyte r, cbyte g, cbyte b)
{
	my1rgb_t temp;
	int *buff = (int*) &temp;
	temp.a = 0; temp.r = b; temp.g = g; temp.b = r;
	/**return (((int)b&0xff)<<16) | (((int)g&0xff)<<8) | ((int)r&0xff);*/
	return *buff;
}
/*----------------------------------------------------------------------------*/
void image_copy_color2bgr(my1image_t *dst, my1image_t *src)
{
	int loop, size = src->length;
	image_make(dst,src->height,src->width);
	if (src->mask==IMASK_COLOR)
	{
		cbyte *that;
		for(loop=0;loop<size;loop++)
		{
			that = (cbyte*) &src->data[loop];
			dst->data[loop] = encode_bgr(that[2],that[1],that[0]);
		}
	}
	else
	{
		cbyte temp;
		for(loop=0;loop<size;loop++)
		{
			temp = (cbyte) src->data[loop];
			dst->data[loop] = encode_bgr(temp,temp,temp);
		}
	}
	dst->mask = IMASK_COLOR;
}
/*----------------------------------------------------------------------------*/
void image_copy_color_channel(my1image_t *dst, my1image_t *src, int mask)
{
	cbyte *that;
	int loop, size = src->length, pick;
	image_make(dst,src->height,src->width);
	switch (mask)
	{
		default:
		case IMASK_COLOR_B: pick = 0; break;
		case IMASK_COLOR_G: pick = 1; break;
		case IMASK_COLOR_R: pick = 2; break;
	}
	for(loop=0;loop<size;loop++)
	{
		that = (cbyte*) &src->data[loop];
		dst->data[loop] = (int) that[pick];
	}
	dst->mask = IMASK_GRAY;
}
/*----------------------------------------------------------------------------*/
int encode_rgb(cbyte r, cbyte g, cbyte b)
{
	my1rgb_t temp;
	int *buff = (int*) &temp;
	temp.a = 0; temp.r = r; temp.g = g; temp.b = b;
	/**return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);*/
	return *buff;
}
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b)
{
	my1rgb_t *buff = (my1rgb_t*) &data;
	*r =  buff->r;
	*g =  buff->g;
	*b =  buff->b;
/**
	*r = (data&0xff0000)>>16;
	*g = (data&0xff00)>>8;
	*b = (data&0xff);
*/
}
/*----------------------------------------------------------------------------*/
int gray2color(int data)
{
	return encode_rgb(data,data,data);
}
/*----------------------------------------------------------------------------*/
int color2gray(int data)
{
	cbyte r, g, b;
	decode_rgb(data,&r,&g,&b);
	/** consider luminosity? */
	/*0.21 R + 0.71 G + 0.07 B*/
	/** go for speed? */
	return gray4rgb(r,g,b);
}
/*----------------------------------------------------------------------------*/
