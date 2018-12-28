/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
/* need the following for laplace2 */
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
/* need for sqrt in sobel */
#include <math.h>
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
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
my1image_t* filter_resize(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	my1image_area_t *size = (my1image_area_t*) filter->data;
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
my1image_t* filter_laplace_1(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_laplace_2(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
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
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	/* +ve gradient when dark(left) -> light(right) */
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	/* +ve gradient when dark(bottom) -> light(top) */
	int coeff[] = { 1,2,1, 0,0,0, -1,-2,-1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	my1image_t buff1, buff2, *chk = (my1image_t*) filter->data;
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
	res->mask = IMASK_GRAY;
	image_limit(res);
	/* clean-up */
	image_free(&buff1);
	image_free(&buff2);
	/* return image structure with containing magnitude */
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gauss(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int coeff[] = { 1,2,1, 2,4,2, 1,2,1};
	image_mask_this(img,res,3,9,coeff);
	image_scale(res,(float)1/16); /* normalize? */
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_maxscale(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int loop, size = img->length, pmax = img->data[0], temp;
	image_make(res,img->height,img->width);
	/* find max value, stretch if less than GRAYLEVEL/2 */
	for (loop=1;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp>pmax)
			pmax = temp;
	}
	if (pmax<GRAYLEVEL>>1)
		pmax = GRAYLEVEL-pmax;
	else pmax = 0;
	for (loop=0;loop<size;loop++)
		res->data[loop] = img->data[loop] + pmax;
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
#define CHECK_TOP 0
#define CHECK_BOTTOM 1
#define CHECK_LEFT 2
#define CHECK_RIGHT 3
#define CHECK_TOP_LEFT 4
#define CHECK_BOTTOM_RIGHT 5
#define CHECK_TOP_RIGHT 6
#define CHECK_BOTTOM_LEFT 7
/*----------------------------------------------------------------------------*/
int image_check_suppress(my1image_t *img, my1image_t *chk, int pref, int pchk)
{
	int pick, temp, edge = img->data[pref];
	switch (pchk)
	{
		default:
		case CHECK_TOP: pick = pref-img->width; break;
		case CHECK_BOTTOM: pick = pref+img->width; break;
		case CHECK_LEFT: pick = pref-1; break;
		case CHECK_RIGHT: pick = pref+1; break;
		case CHECK_TOP_LEFT: pick = pref-img->width-1; break;
		case CHECK_BOTTOM_RIGHT: pick = pref+img->width+1; break;
		case CHECK_TOP_RIGHT: pick = pref-img->width+1; break;
		case CHECK_BOTTOM_LEFT: pick = pref-img->width-1; break;
	}
	/* check valid index */
	if (pick<0||pick>=img->length)
		return edge;
	/* suppress if parallel and stronger edge */
	if (chk->data[pick]==chk->data[pref])
	{
		temp = img->data[pick];
		if (temp>edge)
			edge = 0;
		else
		{
			if (temp==edge)
			{
				/* check further if going 'downwards' */
				switch (pchk)
				{
					case CHECK_BOTTOM:
					case CHECK_RIGHT:
					case CHECK_BOTTOM_RIGHT:
					case CHECK_BOTTOM_LEFT:
						temp = image_check_suppress(img,chk,pick,pchk);
						if (!temp) edge = 0;
						break;
				}
			}
		}
	}
	return edge;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_suppress(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int loop, size = img->length, curr, buff;
	my1image_t *chk = (my1image_t*) filter->data;
	image_make(res,img->height,img->width);
	for (loop=0;loop<size;loop++)
	{
		/* dummy loop */
		do
		{
			/* get current edge value */
			curr = img->data[loop];
			/* skip if no edge? */
			if (!curr) break;
			/* get current edge direction */
			buff = chk->data[loop]; /* edge dir */
			/* check based on edge direction */
			if (buff==0)
			{
				curr = image_check_suppress(img,chk,loop,CHECK_TOP);
				if (!curr) break;
				curr = image_check_suppress(img,chk,loop,CHECK_BOTTOM);
				if (!curr) break;
			}
			else if (buff==90)
			{
				curr = image_check_suppress(img,chk,loop,CHECK_LEFT);
				if (!curr) break;
				curr = image_check_suppress(img,chk,loop,CHECK_RIGHT);
				if (!curr) break;
			}
			else if (buff==45)
			{
				curr = image_check_suppress(img,chk,loop,CHECK_TOP_LEFT);
				if (!curr) break;
				curr = image_check_suppress(img,chk,loop,CHECK_BOTTOM_RIGHT);
				if (!curr) break;
			}
			else if (buff==135)
			{
				curr = image_check_suppress(img,chk,loop,CHECK_TOP_RIGHT);
				if (!curr) break;
				curr = image_check_suppress(img,chk,loop,CHECK_BOTTOM_LEFT);
				if (!curr) break;
			}
		}
		while (0);
		/* assign new value */
		res->data[loop] = curr;
	}
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_threshold(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int loop, size = img->length, temp;
	my1image_histogram_t hist;
	image_get_histogram(img,&hist);
	histogram_get_threshold(&hist);
	image_make(res,img->height,img->width);
	for (loop=0;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp>hist.threshold) temp = WHITE;
		else temp = BLACK;
		res->data[loop] = temp;
	}
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
void image_double_threshold(my1image_t *img, my1image_t *res, int hi, int lo)
{
	int loop, size = img->length;
	int curr, weak = WHITE>>1, that, rows, cols;
	image_make(res,img->height,img->width);
	/* separate into 3 */
	for (loop=0;loop<size;loop++)
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
	res->mask = IMASK_GRAY;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_canny(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	/** WORK IN PROGRESS */
	my1image_buffer_t buff;
	/* initialize buffer stuctures */
	buffer_init(&buff);
	/* should i run gaussian? */
	img = filter_gauss(img,buff.curr,0x0);
	/* calculate directional edge */
	img = filter_sobel(img,buff.next,filter);
	/* non-maximum suppression */
	img = filter_suppress(img,buff.curr,filter);
	/* hi-lo threshold? try-and-error values for now! */
	image_double_threshold(img,res,164,82);
	/* cleanup */
	buffer_free(&buff);
	return res;
}
/*----------------------------------------------------------------------------*/
