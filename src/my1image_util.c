/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
/*----------------------------------------------------------------------------*/
void image_get_region(my1image_t *img, my1image_t *sub, my1region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		yoff = reg->yset;
		row = reg->height;
		xoff = reg->xset;
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
void image_set_region(my1image_t *img, my1image_t *sub, my1region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		yoff = reg->yset;
		row = reg->height;
		xoff = reg->xset;
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
void image_fill_region(my1image_t *img, int val, my1region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		yoff = reg->yset;
		row = reg->height;
		xoff = reg->xset;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImg = image_row_data(img,iloop+yoff);
		for (jloop=0;jloop<col;jloop++)
			pImg[jloop+xoff] = val;
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
void image_mask_make(my1image_mask_t *mask, int size, int *pval)
{
	int loop, temp = mask->length;
	if (size<mask->length) temp = size;
	for (loop=0;loop<temp;loop++)
		mask->factor[loop] = pval[loop];
}
/*----------------------------------------------------------------------------*/
int image_get_valid(my1image_t *image, int row, int col)
{
	if (row<0) row = 0;
	else if (row>=image->height) row = image->height-1;
	if (col<0) col = 0;
	else if (col>=image->width) col = image->width-1;
	return image->data[row*image->width+col];
}
/*----------------------------------------------------------------------------*/
void image_correlation(my1image_t *img, my1image_t *res, my1mask_t *mask)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for (irow=0;irow<img->height;irow++)
	{
		for (icol=0;icol<img->width;icol++)
		{
			value = 0; index = 0;
			for (mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for (mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* cross-correlation */
					value += mask->factor[index++] *
						image_get_valid(img,irow+mrow,icol+mcol);
				}
			}
			image_set_pixel(res,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_convolution(my1image_t *img, my1image_t *res, my1mask_t *mask)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for (irow=0;irow<img->height;irow++)
	{
		for (icol=0;icol<img->width;icol++)
		{
			value = 0; index = 0;
			for (mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for (mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* convolution */
					value += mask->factor[index++] *
						image_get_valid(img,irow-mrow,icol-mcol);
				}
			}
			image_set_pixel(res,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void buffer_init(my1image_buffer_t* ibuff)
{
	ibuff->region.xset = 0;
	ibuff->region.yset = 0;
	ibuff->region.width = 0;
	ibuff->region.height = 0;
	ibuff->curr = &ibuff->main;
	ibuff->next = &ibuff->buff;
	ibuff->temp = 0x0; /* not necessary */
	image_init(ibuff->curr);
	image_init(ibuff->next);
}
/*----------------------------------------------------------------------------*/
void buffer_free(my1image_buffer_t* ibuff)
{
	image_free(ibuff->curr);
	image_free(ibuff->next);
}
/*----------------------------------------------------------------------------*/
void buffer_size(my1image_buffer_t* ibuff, int height, int width)
{
	image_make(ibuff->curr,height,width);
	image_make(ibuff->next,height,width);
}
/*----------------------------------------------------------------------------*/
void buffer_swap(my1image_buffer_t* ibuff)
{
	ibuff->temp = ibuff->curr;
	ibuff->curr = ibuff->next;
	ibuff->next = ibuff->temp;
}
/*----------------------------------------------------------------------------*/
void filter_init(my1image_filter_t* pfilter,
	pfilter_t filter, my1buffer_t *buffer)
{
	pfilter->userdata = 0x0;
	pfilter->buffer = buffer;
	pfilter->docopy = 0x0;
	pfilter->filter = filter;
	pfilter->next = 0x0;
	pfilter->last = 0x0;
}
/*----------------------------------------------------------------------------*/
void filter_free(my1image_filter_t* pfilter)
{
	pfilter->filter = 0x0;
	if (pfilter->next)
		filter_free(pfilter->next);
	pfilter->next = 0x0;
	pfilter->last = 0x0;
}
/*----------------------------------------------------------------------------*/
my1filter_t* filter_insert(my1filter_t* pstack, my1filter_t* pcheck)
{
	pcheck->next = 0x0;
	pcheck->last = 0x0;
	if (!pstack)
	{
		pcheck->last = pcheck;
		return pcheck;
	}
	pstack->last->next = pcheck;
	/* in case pcheck is another stack, look for last */
	while (pcheck->next)
		pcheck = pcheck->next;
	pstack->last = pcheck;
	return pstack;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_filter(my1image_t* image, my1filter_t* pfilter)
{
	while (pfilter)
	{
		if (pfilter->filter&&pfilter->buffer)
		{
			my1image_buffer_t *pbuff = pfilter->buffer;
			pfilter->filter(image,pbuff->next,pfilter->userdata);
			buffer_swap(pbuff);
			image = pbuff->curr;
			/* in case we need a copy of this stage output! */
			if (pfilter->docopy)
			{
				image_make(pfilter->docopy,image->height,image->width);
				image_copy(pfilter->docopy,image);
			}
		}
		pfilter = pfilter->next;
	}
	return image;
}
/*----------------------------------------------------------------------------*/
void image_get_histogram(my1image_t *image, my1histogram_t *hist)
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
void image_smooth_histogram(my1image_t *image, my1histogram_t *hist)
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
