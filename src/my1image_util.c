/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
/*----------------------------------------------------------------------------*/
void image_get_region(my1image_t *img, my1image_t *sub, my1image_region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		xoff = reg->xset;
		yoff = reg->yset;
		row = reg->height;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImg = image_row_data(img,iloop+yoff);
		int* pSub = image_row_data(sub,iloop);
		for (jloop=0;jloop<col;jloop++)
			pSub[jloop] = pImg[jloop+xoff];
	}
}
/*----------------------------------------------------------------------------*/
void image_set_region(my1image_t *img, my1image_t *sub, my1image_region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		xoff = reg->xset;
		yoff = reg->yset;
		row = reg->height;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImg = image_row_data(img,iloop+yoff);
		int* pSub = image_row_data(sub,iloop);
		for (jloop=0;jloop<col;jloop++)
			pImg[jloop+xoff] = pSub[jloop];
	}
}
/*----------------------------------------------------------------------------*/
void image_region_fill(my1image_t *img, int val, my1image_region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		xoff = reg->xset;
		yoff = reg->yset;
		row = reg->height;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImage = image_row_data(img,iloop+yoff);
		for (jloop=0;jloop<col;jloop++)
			pImage[jloop+xoff] = val;
	}
}
/*----------------------------------------------------------------------------*/
int* image_mask_init(my1image_mask_t *mask, int size)
{
	int length = size*size;
	int *temp = (int*) malloc(length*sizeof(int));
	if (temp)
	{
		mask->size = size;
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
void image_mask_make(my1image_mask_t *mask, int *pval)
{
	int index;
	for (index=0;index<mask->length;index++)
		mask->factor[index] = pval[index];
}
/*----------------------------------------------------------------------------*/
int validpixel(my1image_t *image, int row, int col)
{
	if (row<0) row = 0;
	else if (row>=image->height) row = image->height-1;
	if (col<0) col = 0;
	else if (col>=image->width) col = image->width-1;
	return image->data[row*image->width+col];
}
/*----------------------------------------------------------------------------*/
void image_correlation(my1image_t *dst, my1image_t *src, my1image_mask_t *mask)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for (irow=0;irow<dst->height;irow++)
	{
		for (icol=0;icol<dst->width;icol++)
		{
			value = 0; index = 0;
			for (mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for (mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* cross-correlation */
					value += mask->factor[index++] *
						validpixel(src,irow+mrow,icol+mcol);
				}
			}
			image_set_pixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_convolution(my1image_t *dst, my1image_t *src, my1image_mask_t *mask)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for (irow=0;irow<dst->height;irow++)
	{
		for (icol=0;icol<dst->width;icol++)
		{
			value = 0; index = 0;
			for (mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for (mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* convolution */
					value += mask->factor[index++] *
						validpixel(src,irow-mrow,icol-mcol);
				}
			}
			image_set_pixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void filter_init(my1image_filter_t* pfilter, pfilter_t filter)
{
	pfilter->userdata = 0x0;
	image_init(&pfilter->buffer);
	pfilter->filter = filter;
	pfilter->next = 0x0;
}
/*----------------------------------------------------------------------------*/
void filter_free(my1image_filter_t* pfilter)
{
	image_free((void*)&pfilter->buffer);
	pfilter->filter = 0x0;
	if (pfilter->next)
		filter_free(pfilter->next);
	pfilter->next = 0x0;
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* filter_insert(my1image_filter_t* pstack,
	my1image_filter_t* pcheck)
{
	my1image_filter_t *pthis = pstack;
	if (!pstack) return pcheck;
	while (pthis->next) pthis = pthis->next;
	pthis->next = pcheck;
	return pstack;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter)
{
	my1image_t *pcheck = image;
	while (pfilter)
	{
		if (pfilter->filter)
			pcheck = pfilter->filter(pcheck,&pfilter->buffer,
				pfilter->userdata,pfilter->passdata);
		pfilter = pfilter->next;
	}
	return pcheck;
}
/*----------------------------------------------------------------------------*/
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop, temp;
	/* clear histogram */
	for(loop=0;loop<GRAYLEVEL;loop++)
		hist->count[loop] = 0;
	/* count! */
	for(loop=0;loop<image->length;loop++)
	{
		temp = image->data[loop];
		hist->count[temp]++;
	}
	/* get index for highest count */
	hist->maxindex = 0;
	temp = hist->count[0];
	for(loop=1;loop<GRAYLEVEL;loop++)
	{
		if(hist->count[loop]>temp)
		{
			temp = hist->count[loop];
			hist->maxindex = loop;
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop;
	float alpha = (float) WHITE/image->length;
	for(loop=0;loop<image->length;loop++)
	{
		float buff = hist->count[image->data[loop]] * alpha;
		image->data[loop] = (int) buff;
	}
}
/*----------------------------------------------------------------------------*/
