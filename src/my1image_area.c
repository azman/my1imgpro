/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_AREAC__
#define __MY1IMAGE_AREAC__
/*----------------------------------------------------------------------------*/
#include "my1image_area.h"
/*----------------------------------------------------------------------------*/
void image_area_make(my1image_area_t *reg, int y, int x, int h, int w)
{
	reg->yset = y;
	reg->xset = x;
	reg->hval = h;
	reg->wval = w;
}
/*----------------------------------------------------------------------------*/
void image_area_select(my1image_t *img, my1image_area_t *reg, int val, int inv)
{
	int irow, icol, rows = img->rows, cols = img->cols;
	int yoff = 0, xoff = 0, ymax = rows, xmax = cols;
	if (reg)
	{
		yoff = reg->yset;
		ymax = reg->hval+yoff;
		xoff = reg->xset;
		xmax = reg->wval+xoff;
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
	int ysrc = img->rows, xsrc = img->cols;
	int ymax = reg->hval, xmax = reg->wval;
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
void image_get_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
{
	int irow, icol, xoff = 0, yoff = 0;
	int rows = img->rows, cols = img->cols;
	if (reg)
	{
		yoff = reg->yset;
		rows = reg->hval;
		xoff = reg->xset;
		cols = reg->wval;
	}
	for (irow=0;irow<rows;irow++)
	{
		int* pImg = image_row_data(img,irow+yoff);
		int* pSub = image_row_data(sub,irow);
		for (icol=0;icol<cols;icol++)
			pSub[icol] = pImg[icol+xoff];
	}
}
/*----------------------------------------------------------------------------*/
void image_set_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg)
{
	int irow, icol, xoff = 0, yoff = 0;
	int rows = img->rows, cols = img->cols;
	if (reg)
	{
		yoff = reg->yset;
		rows = reg->hval;
		xoff = reg->xset;
		cols = reg->wval;
	}
	for (irow=0;irow<rows;irow++)
	{
		int* pImg = image_row_data(img,irow+yoff);
		int* pSub = image_row_data(sub,irow);
		for (icol=0;icol<cols;icol++)
			pImg[icol+xoff] = pSub[icol];
	}
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_AREAC__ */
/*----------------------------------------------------------------------------*/
