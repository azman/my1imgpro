/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
#include <string.h>
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
void image_get_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
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
void image_set_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
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
void image_area_init(my1image_area_t *reg)
{
	reg->yset = 0;
	reg->xset = 0;
	reg->height = 0;
	reg->width = 0;
}
/*----------------------------------------------------------------------------*/
void image_area_make(my1image_area_t *reg, int y, int x, int h, int w)
{
	reg->yset = y;
	reg->xset = x;
	reg->height = h;
	reg->width = w;
}
/*----------------------------------------------------------------------------*/
void image_area_select(my1image_t *img, my1image_area_t *reg, int val, int inv)
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
				/* outside aoi - fill val if no invert request */
				if (!inv) pImg[icol] = val;
			}
			else
			{
				/* inside aoi - fill val if invert requested */
				if (inv) pImg[icol] = val;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_size_aspect(my1image_t *img, my1image_area_t *reg)
{
	int ysrc = img->height, xsrc = img->width;
	int ymax = reg->height, xmax = reg->width;
	int ychk, xchk;
	/* classify */
	if (ysrc>ymax)
	{
		if (xsrc>xmax&&(((double)xsrc/xmax)>=((double)ysrc/ymax)))
		{
			ychk = xmax*ysrc/xsrc; /* fix x, get new y */
			image_area_make(reg,((ymax-ychk)>>1),0,ychk,xmax);
		}
		else
		{
			xchk = ymax*xsrc/ysrc; /* fix y, get new x */
			image_area_make(reg,0,((xmax-xchk)>>1),ymax,xchk);
		}
	}
	else if (ysrc<ymax)
	{
		if (xsrc<xmax&&(((double)xmax/xsrc)>=((double)ymax/ysrc)))
		{
			xchk = ymax*xsrc/ysrc; /* fix y, get new x */
			image_area_make(reg,0,((xmax-xchk)>>1),ymax,xchk);
		}
		else
		{
			ychk = xmax*ysrc/xsrc; /* fix x, get new y */
			image_area_make(reg,((ymax-ychk)>>1),0,ychk,xmax);
		}
	}
	else if (xsrc!=xmax)
	{
		if (xsrc<xmax)
		{
			xchk = ymax*xsrc/ysrc; /* fix y, get new x */
			image_area_make(reg,0,((xmax-xchk)>>1),ymax,xchk);
		}
		else
		{
			ychk = xmax*ysrc/xsrc; /* fix x, get new y */
			image_area_make(reg,((ymax-ychk)>>1),0,ychk,xmax);
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
	pfilter->buffer = buffer;
	pfilter->output = 0x0;
	pfilter->filter = filter;
	pfilter->doinit = 0x0; /* used in cloning? */
	pfilter->dofree = 0x0;
	pfilter->next = 0x0;
	pfilter->last = 0x0;
}
/*----------------------------------------------------------------------------*/
void filter_free(my1image_filter_t* pfilter)
{
	if (pfilter->dofree)
		pfilter->dofree(pfilter);
}
/*----------------------------------------------------------------------------*/
void filter_unlink(my1image_filter_t* pfilter)
{
	/* simply prepare for new filter chaining */
	pfilter->output = 0x0;
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
	pstack->last = pcheck;
	return pstack;
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* filter_search(my1image_filter_t* ppass, char *name)
{
	int size;
	my1image_filter_t* ipass = 0x0;
	while (ppass)
	{
		if (ppass->name)
		{
			size = strlen(ppass->name);
			if (!strncmp(ppass->name,name,size))
			{
				ipass = ppass;
				break;
			}
		}
		ppass = ppass->next;
	}
	return ipass;
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* filter_clone(my1image_filter_t* ppass)
{
	size_t size = sizeof(my1image_filter_t);
	my1image_filter_t* that = (my1image_filter_t*)malloc(size);
	if (that)
	{
		filter_init(that,ppass->filter,ppass->buffer);
		strncpy(that->name,ppass->name,FILTER_NAMESIZE);
		that->doinit = ppass->doinit;
		that->dofree = ppass->dofree;
		if (that->doinit)
			that->doinit(that);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
void filter_clean(my1image_filter_t* ppass)
{
	my1image_filter_t* ipass;
	while (ppass)
	{
		ipass = ppass;
		ppass = ppass->next;
		filter_free(ipass);
		free((void*)ipass);
	}
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* info_create_filter(filter_info_t* info)
{
	size_t size = sizeof(my1image_filter_t);
	my1image_filter_t* that = (my1image_filter_t*)malloc(size);
	if (that)
	{
		filter_init(that,info->filter,0x0);
		strncpy(that->name,info->name,FILTER_NAMESIZE);
		that->doinit = info->fsetup;
		that->dofree = info->fclean;
		if (that->doinit)
			that->doinit(that);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_filter(my1image_t* image, my1image_filter_t* pfilter)
{
	while (pfilter)
	{
		if (pfilter->filter)
		{
			my1image_buffer_t* pbuff = 0x0;
			my1image_t* check = pfilter->output;
			if (!check)
			{
				/* skip filter if no output and no buffer assigned */
				pbuff = pfilter->buffer;
				if (!pbuff) continue;
				check = pbuff->next;
			}
			image = pfilter->filter(image,check,pfilter);
			/* swap buffer if used */
			if (pbuff) buffer_swap(pbuff);
		}
		pfilter = pfilter->next;
	}
	return image;
}
/*----------------------------------------------------------------------------*/
