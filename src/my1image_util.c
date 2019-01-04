/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
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
void image_correlation(my1image_t *img, my1image_t *res, my1image_mask_t *mask)
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
void image_convolution(my1image_t *img, my1image_t *res, my1image_mask_t *mask)
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
void image_get_region(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
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
void image_set_region(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
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
void image_fill_region(my1image_t *img, my1image_area_t *reg, int val)
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
void image_mask_region(my1image_t *img, my1image_area_t *reg, int inv)
{
	int irow, icol, rows = img->height, cols = img->width;
	int yoff = 0, xoff = 0, ymax = rows, xmax = cols;
	if (reg)
	{
		yoff = reg->yset;
		ymax = reg->height+yoff;
		xoff = reg->xset;
		xmax = reg->width+xoff;
	}
	for (irow=0;irow<rows;irow++)
	{
		int* pImg = image_row_data(img,irow);
		for (icol=0;icol<cols;icol++)
		{
			if ((irow<yoff||irow>=ymax)||(icol<xoff||icol>=xmax))
			{
				/* outside of mask region - mask if invert requested */
				if (inv) pImg[icol] = BLACK;
			}
			else
			{
				/* inside mask region - mask if no invert request */
				if (!inv) pImg[icol] = BLACK;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_size_aspect(my1image_t *img, my1image_area_t *reg)
{
	int rows = img->height, cols = img->width;
	int ymax = reg->height, xmax = reg->width;
	int ychk, xchk;
	/* classify */
	if (rows>xmax) /* assume cols>xmax */
	{
		/* create region bigger than scalable */
		ychk = xmax*rows/cols;
		if (ychk>ymax)
		{
			reg->yset = -((ychk-ymax)>>1);
			reg->height = ychk;
			reg->xset = 0;
			reg->width = xmax;
		}
		else
		{
			xchk = ymax*cols/rows;
			if (xchk>xmax)
			{
				reg->yset = 0;
				reg->height = ymax;
				reg->xset = -((xchk-xmax)>>1);
				reg->width = xchk;
			}
			else
			{
				reg->yset = -((ychk-ymax)>>1);
				reg->height = ymax;
				reg->xset = -((xchk-xmax)>>1);
				reg->width = xmax;
			}
		}
	}
	else if (rows<xmax) /* assume cols<xmax */
	{
		/* create region smaller than scalable */
		ychk = xmax*rows/cols;
		if (ychk<ymax)
		{
			reg->yset = (ymax-ychk)>>1;
			reg->height = ychk;
			reg->xset = 0;
			reg->width = xmax;
		}
		else
		{
			xchk = ymax*cols/rows;
			if (xchk<xmax)
			{
				reg->yset = 0;
				reg->height = ymax;
				reg->xset = (xmax-xchk)>>1;
				reg->width = xchk;
			}
			else
			{
				reg->yset = (ymax-ychk)>>1;
				reg->height = ymax;
				reg->xset = (xmax-cols)>>1;
				reg->width = xmax;
			}
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
	image_init(&ibuff->main);
	image_init(&ibuff->buff);
	image_init(&ibuff->xtra);
}
/*----------------------------------------------------------------------------*/
void buffer_free(my1image_buffer_t* ibuff)
{
	image_free(&ibuff->main);
	image_free(&ibuff->buff);
	image_free(&ibuff->xtra);
}
/*----------------------------------------------------------------------------*/
void buffer_size(my1image_buffer_t* ibuff, int height, int width)
{
	image_make(&ibuff->main,height,width);
	image_make(&ibuff->buff,height,width);
}
/*----------------------------------------------------------------------------*/
void buffer_size_all(my1image_buffer_t* ibuff, int height, int width)
{
	image_make(&ibuff->main,height,width);
	image_make(&ibuff->buff,height,width);
	image_make(&ibuff->xtra,height,width);
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
	pfilter_t filter, my1image_buffer_t* buffer)
{
	pfilter->name[0] = 0x0; /* anonymous */
	pfilter->data = 0x0;
	pfilter->parent = 0x0;
	pfilter->buffer = buffer;
	pfilter->docopy = 0x0;
	pfilter->filter = filter;
	pfilter->next = 0x0;
	pfilter->last = 0x0;
}
/*----------------------------------------------------------------------------*/
void filter_free(my1image_filter_t* pfilter)
{
	/* no resource to release, just prepare for new filter chaining */
	pfilter->docopy = 0x0;
	if (pfilter->next)
		filter_free(pfilter->next);
	pfilter->next = 0x0;
	pfilter->last = 0x0;
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* filter_insert(my1image_filter_t* pstack,
	my1image_filter_t* pcheck)
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
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter)
{
	while (pfilter)
	{
		if (pfilter->filter&&pfilter->buffer)
		{
			my1image_buffer_t *pbuff = pfilter->buffer;
			image = pfilter->filter(image,pbuff->next,pfilter);
			buffer_swap(pbuff);
			/* just in case... should we break? */
			if (!image) image = pbuff->curr;
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
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop, temp;
	/* clear histogram */
	for (loop=0;loop<GRAYLEVEL;loop++)
		hist->count[loop] = 0;
	/* count! */
	for (loop=0;loop<image->length;loop++)
	{
		temp = image->data[loop];
		hist->count[temp]++;
	}
	/* get index for highest/lowest count */
	temp = hist->count[0];
	hist->chkvalue = temp;
	hist->chkindex = -1;
	hist->maxvalue = temp;
	hist->maxindex = 0;
	hist->minvalue = temp;
	hist->minindex = 0;
	for (loop=1;loop<GRAYLEVEL;loop++)
	{
		temp = hist->count[loop];
		if (temp>hist->maxvalue)
		{
			hist->chkvalue = hist->maxvalue;
			hist->chkindex = hist->maxindex;
			hist->maxvalue = temp;
			hist->maxindex = loop;
		}
		/* in case first value IS maximum value */
		if (temp>hist->chkvalue)
		{
			if (loop!=hist->maxindex)
			{
				hist->chkvalue = temp;
				hist->chkindex = loop;
			}
		}
		/* look for non-zero minimum count */
		if (temp<hist->minvalue||!hist->minvalue)
		{
			hist->minvalue = temp;
			hist->minindex = loop;
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop, size = image->length, buff[GRAYLEVEL];
	float alpha = (float)WHITE/image->length;
	for (loop=1;loop<GRAYLEVEL;loop++)
		buff[loop] = (int)(hist->count[loop]*alpha);
	for(loop=0;loop<size;loop++)
		image->data[loop] = buff[image->data[loop]];
}
/*----------------------------------------------------------------------------*/
void histogram_get_threshold(my1image_histogram_t *hist)
{
	int loop, last, init = 0, ends = GRAYLEVEL-1, mids = (init+ends)>>1;
	int stop = GRAYLEVEL>>1, chkl = 0, chkr = 0, temp;
	/* prepare weights, limits (mids in right side) */
	for(loop=init,last=ends;loop<stop;loop++,last--)
	{
		chkl += hist->count[loop];
		chkr += hist->count[last];
	}
	/* balanced histogram thresholding */
	while (init<=ends)
	{
		if (chkr>chkl)
		{
			while (!(temp=hist->count[ends--]));
			chkr -= temp;
			if (((init+ends)>>1)<mids)
			{
				mids--;
				chkr += hist->count[mids];
				chkl -= hist->count[mids];
			}
		}
		else /**if (chkl>=chkr)*/
		{
			while (!(temp=hist->count[init++]));
			chkl -= temp;
			if (((init+ends)>>1)>=mids)
			{
				chkl += hist->count[mids];
				chkr -= hist->count[mids];
				mids++;
			}
		}
	}
	hist->threshold = mids;
}
/*----------------------------------------------------------------------------*/
