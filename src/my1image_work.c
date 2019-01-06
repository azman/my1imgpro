/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
/* needed for sqrt in sobel */
#include <math.h>
/* needed in image_work_create */
#include <stdlib.h>
#include <string.h>
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
void filter_resize_init(my1image_filter_t* filter)
{
	filter->data = malloc(sizeof(my1image_area_t));
	if (filter->data)
		image_area_init((my1image_area_t*)filter->data);
}
/*----------------------------------------------------------------------------*/
void filter_resize_free(my1image_filter_t* filter)
{
	if (filter->data)
		free((void*)filter->data);
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
my1image_t* filter_laplace(my1image_t* img, my1image_t* res,
	my1image_filter_t* filter)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
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
	/* assign to default if not specified */
	if (!chk&&filter->buffer) chk = &filter->buffer->xtra;
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
	/* find max value, stretch to maximum */
	for (loop=1;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp>pmax)
			pmax = temp;
	}
	pmax = GRAYLEVEL-pmax-1;
	for (loop=0;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp>BLACK) temp += pmax;
		res->data[loop] = temp;
	}
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
	/* assign to default if not specified */
	if (!chk&&filter->buffer) chk = &filter->buffer->xtra;
	if (!chk) return img;
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
static const filter_info_t MY1_IFILTER_DB[] =
{
	{ IFNAME_GRAYSCALE, filter_gray, 0x0, 0x0 },
	{ IFNAME_RESIZE, filter_resize, filter_resize_init, filter_resize_free },
	{ IFNAME_LAPLACE, filter_laplace, 0x0, 0x0 },
	{ IFNAME_SOBELX, filter_sobel_x, 0x0, 0x0 },
	{ IFNAME_SOBELY, filter_sobel_y, 0x0, 0x0 },
	{ IFNAME_SOBEL, filter_sobel, 0x0, 0x0 },
	{ IFNAME_GAUSS, filter_gauss, 0x0, 0x0 },
	{ IFNAME_MAXSCALE, filter_maxscale, 0x0, 0x0 },
	{ IFNAME_SUPPRESS, filter_suppress, 0x0, 0x0 },
	{ IFNAME_THRESHOLD, filter_threshold, 0x0, 0x0 }
};
/*----------------------------------------------------------------------------*/
const int IFILTER_DB_SIZE = sizeof(MY1_IFILTER_DB)/sizeof(filter_info_t);
/*----------------------------------------------------------------------------*/
my1image_filter_t* image_work_create(char* name)
{
	my1image_filter_t* that = 0x0;
	filter_info_t* info;
	int loop;
	for(loop=0;loop<IFILTER_DB_SIZE;loop++)
	{
		info = (filter_info_t*)&MY1_IFILTER_DB[loop];
		if(!strncmp(name,info->name,FILTER_NAMESIZE))
		{
			that = info_create_filter(info);
			break;
		}
	}
	return that;
}
/*----------------------------------------------------------------------------*/
my1image_filter_t* image_work_create_all(void)
{
	my1image_filter_t *that = 0x0, *temp;
	filter_info_t* info;
	int loop;
	for(loop=0;loop<IFILTER_DB_SIZE;loop++)
	{
		info = (filter_info_t*)&MY1_IFILTER_DB[loop];
		temp = info_create_filter(info);
		if (temp)
			that = filter_insert(that,temp);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
