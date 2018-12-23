/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
#include "my1image_util.h"
/* heed the following for laplace2 */
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
#include <math.h>
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res, void* data)
{
	int loop;
	image_make(res,img->height,img->width);
	if (img->mask==IMASK_COLOR)
	{
		for(loop=0;loop<img->length;loop++)
			res->data[loop] = color2gray(img->data[loop]);
	}
	else
	{
		for (loop=0;loop<img->length;loop++)
			res->data[loop] = img->data[loop];
	}
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_size_down(my1image_t* img,my1image_t* res,
	int height, int width)
{
	int rows, cols, loop = 0;
	int irow, icol, temp, chkg, chkb;
	my1image_buffer_t buff;
	/* prepare buffer - main/buff for color sum, xtra for count*/
	buffer_init(&buff);
	buffer_size_all(&buff,height,width);
	if (img->mask==IMASK_COLOR)
	{
		/* used for color channels */
		image_fill(&buff.main,0);
		image_fill(&buff.buff,0);
	}
	image_fill(&buff.xtra,0);
	/* prepare output image */
	image_make(res,height,width);
	image_fill(res,0);
	/* browse all rows and cols in img */
	for(rows=0;rows<img->height;rows++)
	{
		for(cols=0;cols<img->width;cols++,loop++)
		{
			/* check where this pixel fits in the smaller image */
			irow = rows*res->height/img->height;
			icol = cols*res->width/img->width;
			/* validate position */
			if (irow>=res->height||icol>=res->width)
				continue;
			/* get index */
			temp = irow*res->width+icol;
			if (img->mask==IMASK_COLOR)
			{
				cbyte r,g,b;
				decode_rgb(img->data[loop],&r,&g,&b);
				res->data[temp] += (int)r;
				buff.main.data[temp] += (int)g;
				buff.buff.data[temp] += (int)b;
			}
			else
			{
				res->data[temp] += img->data[loop];
			}
			buff.xtra.data[temp]++;
		}
	}
	res->mask = img->mask;
	/* update all rows and cols in res */
	for(loop=0;loop<res->length;loop++)
	{
		/* get average */
		temp = res->data[loop] / buff.xtra.data[loop];
		if (temp>WHITE) temp = WHITE;
		if (res->mask==IMASK_COLOR)
		{
			chkg = buff.main.data[loop] / buff.xtra.data[loop];
			if (chkg>WHITE) chkg = WHITE;
			chkb = buff.buff.data[loop] / buff.xtra.data[loop];
			if (chkb>WHITE) chkb = WHITE;
			/* reassign color */
			temp = encode_rgb((cbyte)temp,(cbyte)chkg,(cbyte)chkb);
		}
		res->data[loop] = temp;
	}
	buffer_free(&buff);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_size_this(my1image_t* img,my1image_t* res,
	int height, int width)
{
	if (img->height<=height&&img->width<=width)
		return img;
	return image_size_down(img,res,height,width);
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_resize(my1image_t* img, my1image_t* res, void* data)
{
	my1image_region_t *size = (my1image_region_t*) data;
	return image_size_this(img,res,size->height,size->width);
}
/*----------------------------------------------------------------------------*/
my1image_t* image_mask_this(my1image_t* img, my1image_t* res,
	int mask_size, int data_size, int* pdata)
{
	my1image_mask_t mask;
	if (!image_mask_init(&mask,mask_size))
		return img;
	image_mask_make(&mask,data_size,pdata);
	image_make(res,img->height,img->width);
	image_correlation(img,res,&mask);
	image_mask_free(&mask);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_laplace_1(my1image_t* img, my1image_t* res, void* data)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_laplace_2(my1image_t* img, my1image_t* res, void* data)
{
	my1frame_t buff1, buff2;
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1frame_kernel_t kernel;
	if (!frame_kernel_init(&kernel,3))
		return img;
	image_make(res,img->height,img->width);
	frame_kernel_make(&kernel,9,coeff);
	frame_init(&buff1);
	frame_init(&buff2);
	if (frame_make(&buff1,img->height,img->width)&&
		frame_make(&buff2,img->height,img->width))
	{
		frame_set_image(&buff1,img,0);
		frame_correlate(&buff1,&buff2,&kernel);
		frame_get_image(&buff2,res,1);
	}
	frame_free(&buff1);
	frame_free(&buff2);
	frame_kernel_free(&kernel);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res, void* data)
{
	/* +ve gradient when dark(left) -> light(right) */
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res, void* data)
{
	/* +ve gradient when dark(bottom) -> light(top) */
	int coeff[] = { 1,2,1, 0,0,0, -1,-2,-1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel(my1image_t* img, my1image_t* res, void* data)
{
	my1image_t buff1, buff2, *chk = (my1image_t*) data;
	int irow, icol, x, y, z, p;
	/* initialize buffer structures */
	image_init(&buff1);
	image_init(&buff2);
	/* create temporary buffers */
	if (!image_make(&buff1,img->height,img->width)||
		!image_make(&buff2,img->height,img->width))
	{
		image_free(&buff1);
		image_free(&buff2);
		return img;
	}
	/* calculate directional edge */
	filter_sobel_x(img,&buff1,0x0);
	filter_sobel_y(img,&buff2,0x0);
	/* prepare resulting image structure */
	image_make(res,img->height,img->width);
	if (chk)
	{
		image_make(chk,img->height,img->width);
		chk->mask = IMASK_GRAY;
	}
	/* calculate magnitude & phase for 3x3 neighbourhood */
	for (irow=0;irow<img->height;irow++)
	{
		for (icol=0;icol<img->width;icol++)
		{
			x = image_get_pixel(&buff1,irow,icol);
			y = image_get_pixel(&buff2,irow,icol);
			/* can we do this WITHOUT the sqrt? */
			z = (int)sqrt((y*y)+(x*x));
			if (z>WHITE) z = WHITE;
			image_set_pixel(res,irow,icol,z);
			if (!chk) continue;
			/* assign phase - use logic instead of (int)atan2(y,x) :p */
			/* get 4 angles only: 0, 45, 90, 135! */
			p = 0;
			if (z!=0)
			{
				/* make y positive */
				if (y<0) y = -y;
				/* check first quadrant */
				if (x>0)
				{
					if (y > (x<<1))
					{
						p = 0; /* y axis gradient -> x axis edge! */
					}
					else if (x > (y<<1))
					{
						p = 90; /* x axis gradient -> y axis edge! */
					}
					else
					{
						p = 45;
					}
				}
				else if (x<0)
				{
					x = -x;
					if (y > (x<<1))
					{
						p = 0; /* y axis gradient -> x axis edge! */
					}
					else if (x > (y<<1))
					{
						p = 90; /* x axis gradient -> y axis edge! */
					}
					else
					{
						p = 135;
					}
				}
				else /* only y! */
				{
					if (y)
					{
						p = 0; /* y axis gradient -> x axis edge! */
					}
					else
					{
						/* cannot be!! both x&y cannot be zero! */
						p = 0;
					}
				}
			}
			image_set_pixel(chk,irow,icol,p);
		}
	}
	image_limit(res);
	res->mask = IMASK_GRAY;
	/* clean-up */
	image_free(&buff1);
	image_free(&buff2);
	/* return image structure with containing magnitude */
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gauss(my1image_t* img, my1image_t* res, void* data)
{
	int coeff[] = { 1,2,1, 2,4,2, 1,2,1};
	image_mask_this(img,res,3,9,coeff);
	image_scale(res,(float)1/16); /* normalize? */
	return res;
}
/*----------------------------------------------------------------------------*/
int image_check_suppress(my1image_t *img, my1image_t *chk, int pref, int pchk)
{
	int edge = img->data[pref];
	/* check valid index */
	if (pchk<0||pchk>=img->length||!edge)
		return edge;
	/* suppress if parallel and stronger edge */
	if (chk->data[pchk]==chk->data[pref]&&img->data[pchk]>edge)
		edge = 0;
	return edge;
}
/*----------------------------------------------------------------------------*/
void image_non_max_suppress(my1image_t *img, my1image_t *res, my1image_t *chk)
{
	int loop, curr, buff;
	for (loop=0;loop<img->length;loop++)
	{
		/* get current edge value */
		curr = img->data[loop];
		/* skip if no edge? */
		if (!curr) continue;
		/* get current edge direction */
		buff = chk->data[loop]; /* edge dir */
		/* dummy loop */
		do
		{
			/* check based on edge direction */
			if (buff==0)
			{
				/* get/check top index */
				curr = image_check_suppress(img,chk,loop,loop-img->width);
				if (!curr) break;
				/* get/check bottom index */
				curr = image_check_suppress(img,chk,loop,loop+img->width);
				if (!curr) break;
			}
			else if (buff==90)
			{
				/* get/check left index */
				curr = image_check_suppress(img,chk,loop,loop-1);
				if (!curr) break;
				/* get/check right index */
				curr = image_check_suppress(img,chk,loop,loop+1);
				if (!curr) break;
			}
			else if (buff==45)
			{
				/* get/check top-left index */
				curr = image_check_suppress(img,chk,loop,loop-img->width-1);
				if (!curr) break;
				/* get/check bottom-right index */
				curr = image_check_suppress(img,chk,loop,loop+img->width+1);
				if (!curr) break;
			}
			else if (buff==135)
			{
				/* get/check top-right index */
				curr = image_check_suppress(img,chk,loop,loop-img->width+1);
				if (!curr) break;
				/* get/check bottom-left index */
				curr = image_check_suppress(img,chk,loop,loop+img->width-1);
				if (!curr) break;
			}
		}
		while (0);
		/* assign new value */
		res->data[loop] = curr;
	}
}
/*----------------------------------------------------------------------------*/
void image_double_threshold(my1image_t *img, my1image_t *res, int hi, int lo)
{
	int loop, curr, weak = WHITE>>1, that, rows, cols;
	/* separate into 3 */
	for (loop=0;loop<img->length;loop++)
	{
		/* get current edge value */
		curr = img->data[loop];
		/* sort that */
		if (curr>hi) curr = WHITE; /* strong edge */
		else if (curr>lo) curr = weak; /* weak edge */
		else curr = BLACK; /* no edge */
		/* assign new value */
		res->data[loop] = curr;
	}
	/* suppress weak edge if not connected to strong one */
	for (loop=0;loop<res->length;loop++)
	{
		curr = res->data[loop];
		if (curr!=weak) continue;
		that = loop-res->width-1;
		for (rows=0;rows<3;rows++)
		{
			for (cols=0;cols<3;cols++,that++)
			{
				if (that<0||that>=res->length||that==loop)
					continue;
				if (res->data[that]==WHITE)
				{
					curr = WHITE;
					break;
				}
			}
			that += res->width;
		}
		if (curr!=WHITE) curr = BLACK;
		res->data[loop] = curr;
	}
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_canny(my1image_t* img, my1image_t* res, void* data)
{
	my1image_buffer_t buff;
	/* should i run gaussian? */
	/* initialize buffer stuctures */
	buffer_init(&buff);
	/* create temporary buffers */
	buffer_size(&buff,img->height,img->width);
	/* create the extra buffer */
	image_make(&buff.xtra,img->height,img->width);
	buff.xtra.mask = IMASK_GRAY;
	/* prepare resulting image structure */
	image_make(res,img->height,img->width);
	res->mask = IMASK_GRAY;
	/* calculate directional edge */
	//filter_sobel(img,&buff.main,(void*)&buff.buff);
	filter_sobel(img,res,0x0);
	/* clip weak edges */
	//image_cliplo(&buff.main,WHITE/10);
	image_cliplo(res,WHITE/5);
	/* non-maximum suppression */
	//image_non_max_suppress(&buff.main,&buff.xtra,&buff.buff);
	//image_non_max_suppress(&buff.main,res,&buff.buff);
	/* hi-lo threshold? try-and-error values for now! */
	//image_double_threshold(img,res,164,82);
	/* cleanup */
	buffer_free(&buff);
	return res;
}
/*----------------------------------------------------------------------------*/
