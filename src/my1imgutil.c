/*----------------------------------------------------------------------------*/
#include "my1imgutil.h"
#include <stdlib.h> /* for malloc and free? */
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
int* createmask(my1Mask *mask, int size)
{
	int length = size*size;
	int *temp = (int*) malloc(length*sizeof(int));
	if(temp)
	{
		mask->size = size;
		mask->length = length;
		mask->origin = size/2;
		mask->factor = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void freemask(my1Mask *mask)
{
	free((void*)mask->factor);
	mask->factor = 0x0;
}
/*----------------------------------------------------------------------------*/
void setmask(my1Mask *mask, int *parray)
{
	int index;
	for(index=0;index<mask->length;index++)
	{
		mask->factor[index] = parray[index];
	}
}
/*----------------------------------------------------------------------------*/
int validpixel(my1Image *image, int row, int col)
{
	if(row<0) row = 0;
	else if(row>=image->height) row = image->height-1;
	if(col<0) col = 0;
	else if(col>=image->width) col = image->width-1;
	return image->data[row*image->width+col];
}
/*----------------------------------------------------------------------------*/
void mask_image(my1Mask *mask, my1Image *src, my1Image *dst)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for(irow=0;irow<dst->height;irow++)
	{
		for(icol=0;icol<dst->width;icol++)
		{
			value = 0; index = 0;
			for(mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for(mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* cross-correlation */
					value += mask->factor[index++] *
						validpixel(src,irow+mrow,icol+mcol);
				}
			}
			setimagepixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void conv_image(my1Mask *mask, my1Image *src, my1Image *dst)
{
	int irow, icol, mrow, mcol;
	int index, value;
	/* main loop */
	for(irow=0;irow<dst->height;irow++)
	{
		for(icol=0;icol<dst->width;icol++)
		{
			value = 0; index = 0;
			for(mrow=-mask->origin;mrow<=mask->origin;mrow++)
			{
				for(mcol=-mask->origin;mcol<=mask->origin;mcol++)
				{
					/* convolution */
					value += mask->factor[index++] *
						validpixel(src,irow-mrow,icol-mcol);
				}
			}
			setimagepixel(dst,irow,icol,value);
		}
	}
}
/*----------------------------------------------------------------------------*/
void filter_init(my1ImgFilter* pfilter, pImgPro filter)
{
	pfilter->userdata = 0x0;
	initimage(&pfilter->buffer);
	pfilter->filter = filter;
	pfilter->next = 0x0;
}
/*----------------------------------------------------------------------------*/
void filter_free(my1ImgFilter* pfilter)
{
	freeimage((void*)&pfilter->buffer);
	if(pfilter->next)
		filter_free(pfilter->next);
}
/*----------------------------------------------------------------------------*/
void filter_image(my1ImgFilter* pfilter, my1Image* image, my1Image* result)
{
	while(pfilter)
	{
		if(pfilter->filter)
			result = pfilter->filter(image,result,pfilter->userdata);
		pfilter = pfilter->next;
	}
}
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
void grayscale_image(my1Image *image)
{
	int loop;
	cbyte r, g, b;
	if(image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
		{
			decode_rgb(image->data[loop],&r,&g,&b);
			image->data[loop] = (((unsigned int)r+g+b)/3)&0xFF;
			/** consider luminosity? */
			/*0.21 R + 0.71 G + 0.07 B*/
		}
		image->mask = 0;
	}
	else
	{
		image->mask = 0; /* just remove formality? */
	}
}
/*----------------------------------------------------------------------------*/
int encode_rgb(cbyte r, cbyte g, cbyte b)
{
	return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);
}
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b)
{
	*r = (data&0xff0000)>>16;
	*g = (data&0xff00)>>8;
	*b = (data&0xff);
}
/*----------------------------------------------------------------------------*/
int extract_rgb(my1Image *image, cbyte *rgb)
{
	int mask = 0x0, loop, index = 0;
	if(image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
		{
			decode_rgb(image->data[loop],&rgb[index+2],
				&rgb[index+1],&rgb[index+0]);
			index += 3;
		}
		mask = image->mask;
	}
	else
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
int assign_rgb(my1Image *image, cbyte *rgb)
{
	int mask = 0x0, loop, index = 0;
	char chkr, chkg, chkb;
	if(image->mask==IMASK_COLOR24)
	{
		for(loop=0;loop<image->length;loop++)
		{
			image->data[loop] = encode_rgb(rgb[index+2],
				rgb[index+1],rgb[index+0]);
			index += 3;
		}
		mask = image->mask;
	}
	else
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
