/*----------------------------------------------------------------------------*/
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc & free */
/*----------------------------------------------------------------------------*/
void frame_init(my1frame_t *frame)
{
	frame->width = 0;
	frame->height = 0;
	frame->length = 0;
	frame->data = 0x0;
}
/*----------------------------------------------------------------------------*/
void frame_get_image(my1frame_t *frame, my1image_t *image, int align)
{
	int iloop;
	if (align==1) image_limit(image); /* 0 - 255 => 0.0 - 1.0 */
	for (iloop=0;iloop<image->length;iloop++)
		frame->data[iloop] = (float) image->data[iloop]/WHITE;
}
/*----------------------------------------------------------------------------*/
void frame_set_image(my1frame_t *frame, my1image_t *image, int align)
{
	int iloop;
	if (align==1) frame_limitrange(frame); /* 0 - 255 => 0.0 - 1.0 */
	for (iloop=0;iloop<frame->length;iloop++)
		image->data[iloop] = (int) (frame->data[iloop]*WHITE);
}
/*----------------------------------------------------------------------------*/
float* frame_make(my1frame_t *frame, int height, int width)
{
	int length = height*width;
	float *temp = (float*) malloc(length*sizeof(float));
	if (temp)
	{
		frame->data = temp;
		frame->height = height;
		frame->width = width;
		frame->length = length;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void frame_free(my1frame_t *frame)
{
	if (frame->data) free((void*)frame->data);
	frame->data = 0x0;
	frame->length = 0;
}
/*----------------------------------------------------------------------------*/
void frame_copy(my1frame_t *src, my1frame_t *dst)
{
	int iloop;
	for(iloop=0;iloop<dst->length;iloop++)
		dst->data[iloop] = src->data[iloop];
}
/*----------------------------------------------------------------------------*/
float frame_get_pixel(my1frame_t *frame, int row, int col) /* col(x),row(y) */
{
	return frame->data[row*frame->width+col];
}
/*----------------------------------------------------------------------------*/
void frame_set_pixel(my1frame_t *frame, int row, int col,float pixel)
{
	frame->data[row*frame->width+col] = pixel;
}
/*----------------------------------------------------------------------------*/
float* frame_kernel_init(my1frame_kernel_t *kernel, int size)
{
	int length = size*size;
	float *temp = (float*) malloc(length*sizeof(float));
	if (temp)
	{
		kernel->size = size;
		kernel->length = length;
		kernel->origin = size/2;
		kernel->factor = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void frame_kernel_free(my1frame_kernel_t *kernel)
{
	free((void*)kernel->factor);
	kernel->factor = 0x0;
}
/*----------------------------------------------------------------------------*/
void frame_kernel_make(my1frame_kernel_t *kernel, float *farray)
{
	int index=0;
	for (index=0;index<kernel->length;index++)
		kernel->factor[index] = farray[index];
}
/*----------------------------------------------------------------------------*/
void frame_get_minmax(my1frame_t *frame, float* min, float* max)
{
	int iloop;
	float lmin, lmax, temp;
	lmin = frame->data[0];
	lmax = frame->data[0];
	for (iloop=1;iloop<frame->length;iloop++)
	{
		temp = frame->data[iloop];
		if (temp>lmax) lmax = temp;
		else if(temp<lmin) lmin = temp;
	}
	if (max) *max = lmax;
	if (min) *min = lmin;
}
/*----------------------------------------------------------------------------*/
void frame_normalize(my1frame_t *frame)
{
	int iloop;
	float min, max, scale;
	frame_get_minmax(frame,&min,&max);
	scale = max - min;
	for (iloop=0;iloop<frame->length;iloop++)
		frame->data[iloop] = (frame->data[iloop]-min)/scale;
}
/*----------------------------------------------------------------------------*/
void frame_limitrange(my1frame_t *frame)
{
	int iloop;
	for (iloop=0;iloop<frame->length;iloop++)
	{
		if (frame->data[iloop]>1.0) frame->data[iloop] = 1.0;
		else if (frame->data[iloop]<0.0) frame->data[iloop] = 0.0;
	}
}
/*----------------------------------------------------------------------------*/
void frame_absolute(my1frame_t *frame)
{
	int iloop;
	for (iloop=0;iloop<frame->length;iloop++)
		if (frame->data[iloop]<0.0)
			frame->data[iloop] = -frame->data[iloop];
}
/*----------------------------------------------------------------------------*/
void frame_correlate(my1frame_t *src, my1frame_t *dst,
	my1frame_kernel_t *kernel)
{
	int irow, icol, srow, scol, trow, tcol, mrow, mcol, index;
	float value;
	/* main loop */
	for (irow=0;irow<dst->height;irow++)
	{
		for (icol=0;icol<dst->width;icol++)
		{
			value = 0.0; index = 0;
			srow=irow-kernel->origin;
			for (mrow=0;mrow<kernel->size;mrow++)
			{
				if (srow<0) trow = 0;
				else if (srow>=src->height) trow = src->height-1;
				else trow = srow;
				scol = icol-kernel->origin;
				for (mcol=0;mcol<kernel->size;mcol++)
				{
					if (scol<0) tcol = 0;
					else if (scol>=src->width) tcol = src->width-1;
					else tcol = scol;
					value += kernel->factor[index++] *
						frame_get_pixel(src,trow,tcol);
					scol++;
				}
				srow++;
			}
			frame_set_pixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void frame_convolute(my1frame_t *src,my1frame_t *dst,my1frame_kernel_t *kernel)
{
	int irow, icol, srow, scol, trow, tcol, mrow, mcol, index;
	float value;
	/* main loop */
	for (irow=0;irow<dst->height;irow++)
	{
		for (icol=0;icol<dst->width;icol++)
		{
			value = 0.0; index = 0;
			srow=irow+kernel->origin;
			for (mrow=0;mrow<kernel->size;mrow++)
			{
				if (srow<0) trow = 0;
				else if (srow>=src->height) trow = src->height-1;
				else trow = srow;
				scol = icol+kernel->origin;
				for (mcol=0;mcol<kernel->size;mcol++)
				{
					if (scol<0) tcol = 0;
					else if (scol>=src->width) tcol = src->width-1;
					else tcol = scol;
					value += kernel->factor[index++] *
						frame_get_pixel(src,trow,tcol);
					scol--;
				}
				srow--;
			}
			frame_set_pixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
