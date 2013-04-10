/*----------------------------------------------------------------------------*/
#include "my1imgutil.h"
#include <stdlib.h> /* for malloc and free? */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void image2sub(my1Image *image, my1Image *subimage, my1Region *region)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = image->height, col = image->width;
	if(region)
	{
		xoff = region->xset;
		yoff = region->yset;
		row = region->height;
		col = region->width;
	}
	for(iloop=0;iloop<row;iloop++)
	{
		int* pImage = imgrowdata(image,iloop+yoff);
		int* pSubImage = imgrowdata(subimage,iloop);
		for(jloop=0;jloop<col;jloop++)
		{
			pSubImage[jloop] = pImage[jloop+xoff];
		}
	}
}
/*----------------------------------------------------------------------------*/
void sub2image(my1Image *subimage, my1Image *image, my1Region *region)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = image->height, col = image->width;
	if(region)
	{
		xoff = region->xset;
		yoff = region->yset;
		row = region->height;
		col = region->width;
	}
	for(iloop=0;iloop<row;iloop++)
	{
		int* pImage = imgrowdata(image,iloop+yoff);
		int* pSubImage = imgrowdata(subimage,iloop);
		for(jloop=0;jloop<col;jloop++)
		{
			pImage[jloop+xoff] = pSubImage[jloop];
		}
	}
}
/*----------------------------------------------------------------------------*/
void fillregion(my1Image *image, int value, my1Region *region)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = image->height, col = image->width;
	if(region)
	{
		xoff = region->xset;
		yoff = region->yset;
		row = region->height;
		col = region->width;
	}
	for(iloop=0;iloop<row;iloop++)
	{
		int* pImage = imgrowdata(image,iloop+yoff);
		for(jloop=0;jloop<col;jloop++)
		{
			pImage[jloop+xoff] = value;
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int* createmask(my1Mask *mask, int size)
{
	int length = size*size;
	int *temp = (int*) malloc(length*sizeof(int));
	if(temp)
	{
		mask->size = size;
		mask->length = length;
		mask->orig_x = 0;
		mask->orig_y = 0;
		mask->factor = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void freemask(my1Mask *mask)
{
	free(mask->factor);
	mask->factor = 0x0;
}
/*----------------------------------------------------------------------------*/
void setmask(my1Mask *mask, int *parray)
{
	int index;
	for(index=0;index<mask->length;index++ )
	{
		mask->factor[index] = parray[index];
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void filter_image(my1Image *src, my1Image *dst, my1Mask *mask)
{
	int irow, icol, srow, scol, trow, tcol, mrow, mcol, index;
	int value;
	/* main loop */
	for(irow=0;irow<dst->height;irow++)
	{
		for(icol=0;icol<dst->width;icol++)
		{
			value = 0; index = 0;
			srow=irow-mask->orig_x;
			for(mrow=0;mrow<mask->size;mrow++)
			{
				if(srow<0) trow = 0;
				else if(srow>=src->height) trow = src->height-1;
				else trow = srow;
				scol=icol-mask->orig_y;
				for(mcol=0;mcol<mask->size;mcol++)
				{
					if(scol<0) tcol = 0;
					else if(scol>=src->width) tcol = src->width-1;
					else tcol = scol;
					value += mask->factor[index++] * imagepixel(src,trow,tcol);
					scol++;
				}
				srow++;
			}
			setimagepixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void histogram_image(my1Image *image, my1Hist *hist)
{
	int iloop,length;
	length = image->height*image->width;
	/* clear histogram */
	for(iloop=0;iloop<GRAYLEVEL;iloop++)
		hist->count[iloop] = 0;
	/* count! */
	for(iloop=0;iloop<length;iloop++)
	{
		int temp = image->data[iloop];
		hist->count[temp]++;
	}
}
/*----------------------------------------------------------------------------*/
void histogram_smooth(my1Image *image, my1Hist *hist)
{
	int iloop,length;
	float alpha, buff;
	length = image->height*image->width;
	alpha = (float) WHITE/length;
	for(iloop=0;iloop<length;iloop++)
	{
		int temp = image->data[iloop];
		buff = hist->count[temp] * alpha;
		image->data[iloop] = (int) buff;
	}
}
/*----------------------------------------------------------------------------*/
int histogram_maxindex(my1Hist *hist)
{
	int iloop, temp, max, index = 0;
	max = hist->count[index];
	for(iloop=1;iloop<GRAYLEVEL;iloop++)
	{
		temp = hist->count[iloop];
		if(temp>max)
		{
			temp = max;
			index = iloop;
		}
	}
	return index;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int encode_rgb(char r, char g, char b)
{
	return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);
}
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, char *r, char *g, char *b)
{
	*r = (data&0xff0000)>>16;
	*g = (data&0xff00)>>8;
	*b = (data&0xff);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int extract_rgb(my1Image *image, char *rgb)
{
	int mask = 0x0, loop, index = 0;
	if(image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
		{
			decode_rgb(image->data[loop],&rgb[index+0],
				&rgb[index+1],&rgb[index+2]);
			index += 3;
		}
		mask = image->mask;
	}
	else if(image->mask==IMASK_GRAY8)
	{
		for(loop=0;loop<image->length;loop++)
		{
			rgb[index++] = (image->data[loop]&0xff);
			rgb[index++] = (image->data[loop]&0xff);
			rgb[index++] = (image->data[loop]&0xff);
		}
		mask = image->mask;
	}
	return mask;
}
/*----------------------------------------------------------------------------*/
int assign_rgb(my1Image *image, char *rgb)
{
	int mask = 0x0, loop, index = 0;
	char chkr, chkg, chkb;
	if(image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
		{
			image->data[loop] = encode_rgb(rgb[index+0],
				rgb[index+1],rgb[index+2]);
			index += 3;
		}
		mask = image->mask;
	}
	else if(image->mask==IMASK_GRAY8)
	{
		for(loop=0;loop<image->length;loop++)
		{
			chkr = rgb[index++];
			chkg = rgb[index++];
			chkb = rgb[index++];
			image->data[loop] = ((int)chkr+chkg+chkb)/3;
		}
		mask = image->mask;
	}
	return mask;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
