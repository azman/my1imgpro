/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
#include "my1image_util.h"
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
#include <math.h>
/*----------------------------------------------------------------------------*/
my1image_t* filter_gray(my1image_t* img, my1image_t* res, void* userdata)
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
my1image_t* filter_laplace_1(my1image_t* img, my1image_t* res, void* userdata)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	return image_mask_this(img,res,3,9,coeff);
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_laplace_2(my1image_t* img, my1image_t* res, void* userdata)
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
my1image_t* filter_sobel_x(my1image_t* img, my1image_t* res, void* userdata)
{
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	return image_mask_this(img,res,3,9,coeff);
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel_y(my1image_t* img, my1image_t* res, void* userdata)
{
	int coeff[] = { -1,-2,-1, 0,0,0, 1,2,1 };
	return image_mask_this(img,res,3,9,coeff);
}
/*----------------------------------------------------------------------------*/
int iabs(int value)
{
	if (value<0) value = -value;
	return value;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_sobel(my1image_t* img, my1image_t* res, void* userdata)
{
	my1image_t buff1, buff2, *pphase = (my1image_t*) userdata;
	int irow, icol, x, y;
	/* initialize buffer stuctures */
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
	if (pphase)
	{
		image_make(pphase,img->height,img->width);
		pphase->mask = IMASK_GRAY;
	}
	/* calculate magnitude & phase for 3x3 neighbourhood */
	for (irow=0;irow<img->height;irow++)
	{
		for (icol=0;icol<img->width;icol++)
		{
			x = image_get_pixel(&buff1,irow,icol);
			y = image_get_pixel(&buff2,irow,icol);
			image_set_pixel(res,irow,icol,(int)sqrt((y*y)+(x*x)));
			if (!pphase) continue;
			image_set_pixel(pphase,irow,icol,(int)atan2(y,x));
		}
	}
	res->mask = IMASK_GRAY;
	/* clean-up */
	image_free(&buff1);
	image_free(&buff2);
	/* return image structure with containing magnitude */
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* filter_gauss(my1image_t* img, my1image_t* res, void* userdata)
{
	int coeff[] = { 1,4,7,4,1, 4,16,26,16,4,
		7,26,41,26,7, 4,16,26,16,4, 1,4,7,4,1};
	image_mask_this(img,res,5,25,coeff);
	image_scale(res,(1.0/273.0)); /* normalize? */
	return res;
}
/*----------------------------------------------------------------------------*/
