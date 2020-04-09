/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_CRGBC__
#define __MY1IMAGE_CRGBC__
/*----------------------------------------------------------------------------*/
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
int encode_bgr(cbyte r, cbyte g, cbyte b)
{
	my1rgb_t temp;
	int *buff = (int*) &temp;
	/* exclusive for my1image_view_draw - need alpha channel to be 255! */
	temp.a = 0xff; temp.r = b; temp.g = g; temp.b = r;
	return *buff;
}
/*----------------------------------------------------------------------------*/
int color_swap(int data)
{
	my1rgb_t *buff = (my1rgb_t*) &data;
	int temp = buff->r;
	buff->r = buff->b;
	buff->b = (cbyte) temp;
	return data;
}
/*----------------------------------------------------------------------------*/
int image_make_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, size = image->size;
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
int image_form_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
	{
		decode_rgb(image->data[loop],&rgb[0],&rgb[1],&rgb[2]);
		rgb += 3;
	}
	image->mask = IMASK_COLOR;
	return image->mask;
}
/*----------------------------------------------------------------------------*/
void image_grayscale(my1image_t *image)
{
	if (image->mask==IMASK_COLOR)
	{
		int loop, size = image->size;
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
		int loop, size = image->size;
		for (loop=0;loop<size;loop++)
			image->data[loop] = gray2color(image->data[loop]);
		image->mask = IMASK_COLOR;
	}
}
/*----------------------------------------------------------------------------*/
void image_invert_this(my1image_t *image)
{
	if (image->mask==IMASK_COLOR)
	{
		cbyte r, g, b;
		int loop, size = image->size;
		for(loop=0;loop<size;loop++)
		{
			decode_rgb(image->data[loop],&r,&g,&b);
			r = WHITE - r; g = WHITE - g; b = WHITE - b;
			image->data[loop] = encode_rgb(r,g,b);
		}
	}
	else image_invert(image);
}
/*----------------------------------------------------------------------------*/
void image_copy_color2bgr(my1image_t *dst, my1image_t *src)
{
	int loop, size = src->size;
	image_make(dst,src->rows,src->cols);
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
void image_copy_color2rgb(my1image_t *dst, my1image_t *src)
{
	int loop, size = src->size;
	image_make(dst,src->rows,src->cols);
	if (src->mask==IMASK_COLOR)
	{
		cbyte *that;
		for(loop=0;loop<size;loop++)
		{
			that = (cbyte*) &src->data[loop];
			dst->data[loop] = encode_rgb(that[0],that[1],that[2]);
		}
	}
	else
	{
		cbyte temp;
		for(loop=0;loop<size;loop++)
		{
			temp = (cbyte) src->data[loop];
			dst->data[loop] = encode_rgb(temp,temp,temp);
		}
	}
	dst->mask = IMASK_COLOR;
}
/*----------------------------------------------------------------------------*/
void image_copy_color_channel(my1image_t *dst, my1image_t *src, int mask)
{
	cbyte *that;
	int loop, size = src->size, pick;
	image_make(dst,src->rows,src->cols);
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
my1image_t* image_size_size(my1image_t* image, my1image_t* check,
	int height, int width)
{
	int rows, cols, loop = 0;
	int irow, icol, temp, chkg, chkb;
	int zrow, zcol, trow, tcol;
	my1image_t buff1, buff2, buff3;
	cbyte r,g,b;
	/* prepare buffer - main/buff for color sum, xtra for count*/
	image_init(&buff1);
	image_init(&buff2);
	image_init(&buff3);
	image_make(&buff1,height,width);
	image_make(&buff2,height,width);
	image_make(&buff3,height,width);
	if (image->mask==IMASK_COLOR)
	{
		/* used for color channels */
		image_fill(&buff1,0);
		image_fill(&buff2,0);
	}
	image_fill(&buff3,0);
	/* prepare output image */
	image_make(check,height,width);
	image_fill(check,0);
	/* browse all rows and cols in image */
	for(rows=0;rows<image->rows;rows++)
	{
		for(cols=0;cols<image->cols;cols++,loop++)
		{
			/* check where this pixel fits in the target image */
			irow = rows*check->rows/image->rows;
			icol = cols*check->cols/image->cols;
			/* validate position */
			if (irow>=check->rows||icol>=check->cols)
				continue;
			/* get index */
			temp = irow*check->cols+icol;
			if (image->mask==IMASK_COLOR)
			{
				decode_rgb(image->data[loop],&r,&g,&b);
				check->data[temp] += (int)r;
				buff1.data[temp] += (int)g;
				buff2.data[temp] += (int)b;
			}
			else check->data[temp] += image->data[loop];
			/* update count */
			buff3.data[temp]++;
			/* check multiple targets */
			if (check->rows>image->rows||check->cols>image->cols)
			{
				trow = (rows+1)*check->rows/image->rows;
				tcol = (cols+1)*check->cols/image->cols;
				for (zrow=irow;zrow<trow&&zrow<check->rows;zrow++)
				{
					for (zcol=icol;zcol<tcol&&zcol<check->cols;zcol++)
					{
						if (zrow==irow&&zcol==icol) continue;
						temp = zrow*check->cols+zcol;
						if (image->mask==IMASK_COLOR)
						{
							check->data[temp] += (int)r;
							buff1.data[temp] += (int)g;
							buff2.data[temp] += (int)b;
						}
						else check->data[temp] += image->data[loop];
						buff3.data[temp]++;
					}
				}
			}
		}
	}
	check->mask = image->mask;
	/* update all rows and cols in check */
	for(loop=0;loop<check->size;loop++)
	{
		if (!buff3.data[loop]) continue;
		/* get average */
		temp = check->data[loop] / buff3.data[loop];
		if (temp>WHITE) temp = WHITE;
		if (check->mask==IMASK_COLOR)
		{
			chkg = buff1.data[loop] / buff3.data[loop];
			if (chkg>WHITE) chkg = WHITE;
			chkb = buff2.data[loop] / buff3.data[loop];
			if (chkb>WHITE) chkb = WHITE;
			/* reassign color */
			temp = encode_rgb((cbyte)temp,(cbyte)chkg,(cbyte)chkb);
		}
		check->data[loop] = temp;
	}
	image_free(&buff3);
	image_free(&buff2);
	image_free(&buff1);
	return check;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_size_this(my1image_t* image, my1image_t* check,
	int height, int width)
{
	if (image->rows!=height||image->cols!=width)
		return image_size_size(image,check,height,width);
	image_copy(check,image);
	return check;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_CRGBC__ */
/*----------------------------------------------------------------------------*/
