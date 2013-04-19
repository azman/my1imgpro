/*----------------------------------------------------------------------------*/
#include "my1imgfpo.h"
#include <stdlib.h> /* for malloc & free */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void initframe(my1IFrame *frame)
{
	frame->width = 0;
	frame->height = 0;
	frame->length = 0;
	frame->data = 0x0;
}
/*----------------------------------------------------------------------------*/
void image2frame(my1Image *image, my1IFrame *frame, int align)
{
	int iloop,length;
	if(align==1) limit_pixel(image); /* 0 - 255 => 0.0 - 1.0 */
	length = image->height*image->width;
	for(iloop=0;iloop<length;iloop++)
	{
		frame->data[iloop] = (float) image->data[iloop]/WHITE;
	}
}
/*----------------------------------------------------------------------------*/
void frame2image(my1IFrame *frame, my1Image *image, int align)
{
	int iloop,length;
	if(align==1) limitrange_frame(frame); /* 0 - 255 => 0.0 - 1.0 */
	length = frame->height*frame->width;
	for(iloop=0;iloop<length;iloop++)
	{
		image->data[iloop] = (int) (frame->data[iloop]*WHITE);
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
float* createframe(my1IFrame *frame, int height, int width)
{
	int length = height*width;
	float *temp = (float*) malloc(length*sizeof(float));
	if(temp)
	{
		frame->data = temp;
		frame->height = height;
		frame->width = width;
		frame->length = length;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void freeframe(my1IFrame *frame)
{
	if(frame->length) free(frame->data);
	frame->data = 0x0;
	frame->length = 0;
}
/*----------------------------------------------------------------------------*/
void copyframe(my1IFrame *src, my1IFrame *dst)
{
	int iloop;
	for(iloop=0;iloop<dst->length;iloop++)
	{
		dst->data[iloop] = src->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
float framepixel(my1IFrame *frame, int row, int col) /* col(x),row(y) */
{
	return frame->data[row*frame->width+col];
}
/*----------------------------------------------------------------------------*/
void setframepixel(my1IFrame *frame, int row, int col,float pixel)
{
	frame->data[row*frame->width+col] = pixel;
}
/*----------------------------------------------------------------------------*/
float* createkernel(my1Kernel *kernel, int size)
{
	int length = size*size;
	float *temp = (float*) malloc(length*sizeof(float));
	if(temp)
	{
		kernel->size = size;
		kernel->length = length;
		kernel->orig_x = 0;
		kernel->orig_y = 0;
		kernel->factor = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void freekernel(my1Kernel *kernel)
{
	free(kernel->factor);
	kernel->factor = 0x0;
}
/*----------------------------------------------------------------------------*/
void setkernel(my1Kernel *kernel, float *farray)
{
	int index=0;
	for(index=0;index<kernel->length;index++ )
	{
		kernel->factor[index] = farray[index];
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void getminmax_frame(my1IFrame *frame, float* min, float* max)
{
	int iloop;
	float lmin, lmax, temp;
	lmin = frame->data[0];
	lmax = frame->data[0];
	for(iloop=1;iloop<frame->length;iloop++)
	{
		temp = frame->data[iloop];
		if(temp>lmax) lmax = temp;
		else if(temp<lmin) lmin = temp;
	}
	*max = lmax; *min = lmin;
}
/*----------------------------------------------------------------------------*/
void normalize_frame(my1IFrame *frame)
{
	int iloop;
	float min, max, scale;
	getminmax_frame(frame,&min,&max);
	scale = max - min;
	for(iloop=0;iloop<frame->length;iloop++)
	{
		frame->data[iloop] = (frame->data[iloop]-min)/scale;
	}
}
/*----------------------------------------------------------------------------*/
void limitrange_frame(my1IFrame *frame)
{
	int iloop;
	for(iloop=0;iloop<frame->length;iloop++)
	{
		if(frame->data[iloop]>1.0) frame->data[iloop] = 1.0;
		else if(frame->data[iloop]<0.0) frame->data[iloop] = 0.0;
	}
}
/*----------------------------------------------------------------------------*/
void absolute_frame(my1IFrame *frame)
{
	int iloop;
	for(iloop=0;iloop<frame->length;iloop++)
	{
		if(frame->data[iloop]<0.0)
			frame->data[iloop] = -frame->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void correlate_frame(my1IFrame *src, my1IFrame *dst, my1Kernel *kernel)
{
	int irow, icol, srow, scol, trow, tcol, mrow, mcol, index;
	float value;
	/* main loop */
	for(irow=0;irow<dst->height;irow++)
	{
		for(icol=0;icol<dst->width;icol++)
		{
			value = 0.0; index = 0;
			srow=irow-kernel->orig_x;
			for(mrow=0;mrow<kernel->size;mrow++)
			{
				if(srow<0) trow = 0;
				else if(srow>=src->height) trow = src->height-1;
				else trow = srow;
				scol=icol-kernel->orig_y;
				for(mcol=0;mcol<kernel->size;mcol++)
				{
					if(scol<0) tcol = 0;
					else if(scol>=src->width) tcol = src->width-1;
					else tcol = scol;
					value += kernel->factor[index++] * framepixel (src,trow,tcol);
					scol++;
				}
				srow++;
			}
			setframepixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
