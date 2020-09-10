/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_IFDBC__
#define __MY1IMAGE_IFDBC__
/*----------------------------------------------------------------------------*/
#include "my1image_ifdb.h"
#include "my1image_crgb.h"
#include "my1image_mask.h"
#include "my1image_scan.h"
#include "my1image_mono.h"
/*----------------------------------------------------------------------------*/
/* needed for sqrt in sobel */
#include <math.h>
/* needed in image_work_create */
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define FLAG_GRAY FILTER_FLAG_GRAY
#define FLAG_PROG FILTER_FLAG_PROG
/*----------------------------------------------------------------------------*/
my1image_t* filter_binary(my1image_t* img, my1image_t* res, my1ifilter_t* pass)
{
	my1if_options_t *opts = (my1if_options_t*) pass->data;
	image_copy(res,img);
	image_binary(res,opts->par1,WHITE);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
void filter_binary_init(my1ifilter_t* filter, my1ifilter_t* pclone)
{
	my1if_options_t *opts, *temp;
	filter->data = malloc(sizeof(my1if_options_t));
	opts = (my1if_options_t*) filter->data;
	opts->flag = 0;
	if (!pclone)
	{
		opts->par1 = BINARY_CUT;
	}
	else
	{
		temp = (my1if_options_t*) pclone->data;
		opts->par1 = temp->par1;
	}
}
/*----------------------------------------------------------------------------*/
void filter_binary_free(my1ifilter_t* filter)
{
	if (filter->data) free(filter->data);
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_binary_mid(my1image_t* img, my1image_t* res,
	my1ifilter_t* pass)
{
	int loop, pmax, temp, step, size = img->size;
	image_make(res,img->rows,img->cols);
	/* find average value */
	for (loop=0,pmax=0,step=0;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp)
		{
			pmax += temp;
			step++;
		}
		res->data[loop] = temp;
	}
	if (step) pmax /= step;
	image_binary(res,pmax,WHITE);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_morph_erode(my1image_t* img, my1image_t* res,
	my1ifilter_t* pass)
{
	int elem[] = { 1,1,1, 1,1,1, 1,1,1 };
	my1image_t temp;
	image_init(&temp);
	image_copy(&temp,img);
	image_erode(&temp,res,elem);
	image_binary(res,0,WHITE);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_morph_dilate(my1image_t* img, my1image_t* res,
	my1ifilter_t* pass)
{
	int elem[] = { 1,1,1, 1,1,1, 1,1,1 };
	my1image_t temp;
	image_init(&temp);
	image_copy(&temp,img);
	image_dilate(&temp,res,elem);
	image_binary(res,0,WHITE);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res, my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = color2gray(img->data[loop]);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_color_blue(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = (img->data[loop]&IMASK_COLOR_B);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_color_green(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = (img->data[loop]&IMASK_COLOR_G)>>8;
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_color_red(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = (img->data[loop]&IMASK_COLOR_R)>>16;
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray2b(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = (img->data[loop]&IMASK_COLOR_B);
	res->mask = IMASK_COLOR;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray2g(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = ((img->data[loop]<<8)&IMASK_COLOR_G);
	res->mask = IMASK_COLOR;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray2r(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size;
	image_make(res,img->rows,img->cols);
	for(loop=0;loop<size;loop++)
		res->data[loop] = ((img->data[loop]<<16)&IMASK_COLOR_R);
	res->mask = IMASK_COLOR;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_invert(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	image_copy(res,img);
	image_invert_this(res);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_resize(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	my1if_resize_t *size = (my1if_resize_t*) filter->data;
	if (size)
		img = image_size_this(img,&size->buff,size->area.hval,size->area.wval);
	image_copy(res,img);
	return res;
}
/*----------------------------------------------------------------------------*/
void filter_resize_init(my1ifilter_t* filter, my1ifilter_t* pclone)
{
	my1if_resize_t *size, *temp;
	filter->data = malloc(sizeof(my1if_resize_t));
	size = (my1if_resize_t*) filter->data;
	image_init(&size->buff);
	if (!pclone)
		image_area_make(&size->area,0,0,RESIZE_DEF_H,RESIZE_DEF_W);
	else
	{
		temp = (my1if_resize_t*) pclone->data;
		image_area_make(&size->area,0,0,temp->area.hval,temp->area.wval);
	}
}
/*----------------------------------------------------------------------------*/
void filter_resize_free(my1ifilter_t* filter)
{
	my1if_resize_t *size = (my1if_resize_t*) filter->data;
	image_free(&size->buff);
	if (filter->data) free(filter->data);
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_laplace(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	/* +ve gradient when dark(left) -> light(right) */
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	/* +ve gradient when dark(bottom) -> light(top) */
	int coeff[] = { 1,2,1, 0,0,0, -1,-2,-1 };
	image_mask_this(img,res,3,9,coeff);
	image_limit(res);
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int irow, icol, x, y, z, p;
	my1image_t *buff = (my1image_t*) filter->data;
	/* prepare spaces */
	image_make(&buff[0],img->rows,img->cols);
	image_make(&buff[1],img->rows,img->cols);
	image_make(&buff[2],img->rows,img->cols);
	image_make(res,img->rows,img->cols);
	/* calculate directional edge */
	filter_sobel_x(img,&buff[1],0x0);
	filter_sobel_y(img,&buff[2],0x0);
	/* calculate magnitude & phase for 3x3 neighbourhood */
	for (irow=0;irow<img->rows;irow++)
	{
		for (icol=0;icol<img->cols;icol++)
		{
			x = image_get_pixel(&buff[1],irow,icol);
			y = image_get_pixel(&buff[2],irow,icol);
			/* can we do this WITHOUT the sqrt? */
			z = (int)sqrt((y*y)+(x*x));
			if (z>WHITE) z = WHITE;
			image_set_pixel(res,irow,icol,z);
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
			image_set_pixel(&buff[0],irow,icol,p);
		}
	}
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
void filter_sobel_init(my1ifilter_t* filter, my1ifilter_t* pclone)
{
	int loop;
	my1image_t *buff;
	/* prepare 3 buffers for sobel */
	filter->data = malloc(3*sizeof(my1image_t));
	buff = (my1image_t*) filter->data;
	for (loop=0;loop<3;loop++)
		image_init(&buff[loop]);
}
/*----------------------------------------------------------------------------*/
void filter_sobel_free(my1ifilter_t* filter)
{
	int loop;
	my1image_t *buff;
	if (filter->data)
	{
		buff = (my1image_t*) filter->data;
		for (loop=0;loop<3;loop++)
			image_free(&buff[loop]);
		free((void*)filter->data);
	}
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gauss(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int coeff[] = { 1,2,1, 2,4,2, 1,2,1};
	image_mask_this(img,res,3,9,coeff);
	image_scale(res,(float)1/16); /* normalize? */
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_maxscale(my1image_t* img, my1image_t* res,
	my1ifilter_t* filter)
{
	int loop, size = img->size, pmax = img->data[0], temp;
	image_make(res,img->rows,img->cols);
	/* find max value, stretch to maximum */
	for (loop=1;loop<size;loop++)
	{
		temp = img->data[loop];
		if (temp>pmax)
			pmax = temp;
	}
	pmax = WHITE-pmax;
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
		case CHECK_TOP: pick = pref-img->cols; break;
		case CHECK_BOTTOM: pick = pref+img->cols; break;
		case CHECK_LEFT: pick = pref-1; break;
		case CHECK_RIGHT: pick = pref+1; break;
		case CHECK_TOP_LEFT: pick = pref-img->cols-1; break;
		case CHECK_BOTTOM_RIGHT: pick = pref+img->cols+1; break;
		case CHECK_TOP_RIGHT: pick = pref-img->cols+1; break;
		case CHECK_BOTTOM_LEFT: pick = pref-img->cols-1; break;
	}
	/* check valid index */
	if (pick<0||pick>=img->size)
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
	my1ifilter_t* filter)
{
	/** maximal suppression (after sobel?) */
	int loop, curr, buff;
	my1image_t *chk;
	my1ifilter_t *prev = filter->prev;
	if (!prev||strncmp(prev->name,"sobel",6))
	{
		image_copy(res,img);
		return res;
	}
	chk = (my1image_t*) prev->data;
	/* assign to default if not specified */
	image_make(res,img->rows,img->cols);
	for (loop=0;loop<img->size;loop++)
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
my1image_t* filter_threshold(my1image_t* img,my1image_t* res,my1ifilter_t* chk)
{
	my1if_threshold_t* data;
	int loop, temp;
	data = (my1if_threshold_t*) chk->data;
	image_get_histogram(img,&data->hist);
	histogram_get_threshold(&data->hist);
	image_make(res,img->rows,img->cols);
	for (loop=0;loop<img->size;loop++)
	{
		temp = img->data[loop];
		if (temp>data->hist.threshold) temp = WHITE;
		else temp = BLACK;
		res->data[loop] = temp;
	}
	res->mask = IMASK_GRAY;
	return res;
}
/*----------------------------------------------------------------------------*/
void filter_threshold_init(my1ifilter_t* filter, my1ifilter_t* pclone)
{
	filter->data = malloc(sizeof(my1if_threshold_t));
}
/*----------------------------------------------------------------------------*/
void filter_threshold_free(my1ifilter_t* filter)
{
	if (filter->data) free(filter->data);
}
/*----------------------------------------------------------------------------*/
static const filter_info_t MY1_IFILTER_DB[] =
{
	{ IFNAME_BINARY, 0, FLAG_GRAY, filter_binary,
		filter_binary_init, filter_binary_free },
	{ IFNAME_BINARY_MID, 0, FLAG_GRAY, filter_binary_mid, 0x0, 0x0 },
	{ IFNAME_MORPH_ERODE, 0, FLAG_GRAY, filter_morph_erode, 0x0, 0x0 },
	{ IFNAME_MORPH_DILATE, 0, FLAG_GRAY, filter_morph_dilate, 0x0, 0x0 },
	{ IFNAME_GRAYSCALE, 0, 0, filter_gray, 0x0, 0x0 },
	{ IFNAME_COLORBLUE, 0, 0, filter_color_blue, 0x0, 0x0 },
	{ IFNAME_COLORGREEN, 0, 0, filter_color_green, 0x0, 0x0 },
	{ IFNAME_COLORRED, 0, 0, filter_color_red, 0x0, 0x0 },
	{ IFNAME_GRAYBLUE, 0, 0, filter_gray2b, 0x0, 0x0 },
	{ IFNAME_GRAYGREEN, 0, 0, filter_gray2g, 0x0, 0x0 },
	{ IFNAME_GRAYRED, 0, 0, filter_gray2r, 0x0, 0x0 },
	{ IFNAME_INVERT, 0, 0, filter_invert, 0x0, 0x0 },
	{ IFNAME_RESIZE, 0, 0, filter_resize,
		filter_resize_init, filter_resize_free },
	{ IFNAME_LAPLACE, 0, FLAG_GRAY, filter_laplace, 0x0, 0x0 },
	{ IFNAME_SOBELX, 0, FLAG_GRAY, filter_sobel_x, 0x0, 0x0 },
	{ IFNAME_SOBELY, 0, FLAG_GRAY, filter_sobel_y, 0x0, 0x0 },
	{ IFNAME_SOBEL, 0, FLAG_GRAY, filter_sobel,
		filter_sobel_init, filter_sobel_free },
	{ IFNAME_GAUSS, 0, FLAG_GRAY, filter_gauss, 0x0, 0x0 },
	{ IFNAME_MAXSCALE, 0, FLAG_GRAY, filter_maxscale, 0x0, 0x0 },
	{ IFNAME_SUPPRESS, 0, FLAG_GRAY|FLAG_PROG, filter_suppress, 0x0, 0x0 },
	{ IFNAME_THRESHOLD, 0, FLAG_GRAY, filter_threshold,
		filter_threshold_init, filter_threshold_free }
};
/*----------------------------------------------------------------------------*/
const int IFILTER_DB_SIZE = sizeof(MY1_IFILTER_DB)/sizeof(filter_info_t);
/*----------------------------------------------------------------------------*/
filter_info_t* image_work_find_info(char* name)
{
	int loop;
	filter_info_t *info, *find = 0x0;
	for(loop=0;loop<IFILTER_DB_SIZE;loop++)
	{
		info = (filter_info_t*)&MY1_IFILTER_DB[loop];
		if(!strncmp(name,info->name,FILTER_NAMESIZE))
		{
			find = info;
			break;
		}
	}
	return find;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* image_work_create(char* name)
{
	my1ifilter_t* that = 0x0;
	filter_info_t* info = image_work_find_info(name);
	if (info)
		that = info_create_filter(info);
	return that;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* image_work_create_all(void)
{
	int loop;
	filter_info_t* info;
	my1ifilter_t *temp, *that = 0x0;
	for(loop=0;loop<IFILTER_DB_SIZE;loop++)
	{
		info = (filter_info_t*)&MY1_IFILTER_DB[loop];
		temp = info_create_filter(info);
		if (temp) that = filter_insert(that,temp);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_IFDBC__ */
/*----------------------------------------------------------------------------*/
