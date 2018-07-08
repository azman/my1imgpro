/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
#include "my1image_util.h"
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
#include <math.h>
/*----------------------------------------------------------------------------*/
my1image_t* image_mask_this(my1image_t* img,my1image_t* res,int size,int* pdata)
{
	my1image_mask_t mask;
	if (!image_mask_init(&mask,size)) return img;
	image_mask_make(&mask,pdata);
	if (!res->data) image_make(res,img->height,img->width);
	image_correlation(res,img,&mask);
	image_mask_free(&mask);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_laplace(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	return image_mask_this(img,res,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1image_t* image_sobel_x(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
{
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	return image_mask_this(img,res,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1image_t* image_sobel_y(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
{
	int coeff[] = { -1,-2,-1, 0,0,0, 1,2,1 };
	return image_mask_this(img,res,3,coeff);
}
/*----------------------------------------------------------------------------*/
int iabs(int value)
{
	if(value<0) value = -value;
	return value;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_sobel(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
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
	image_sobel_x(img,&buff1,0x0,0x0);
	image_sobel_y(img,&buff2,0x0,0x0);

	/* prepare resulting image structure */
	if (!res->data) image_make(res,img->height,img->width);
	if (pphase&&!pphase->data) image_make(pphase,img->height,img->width);

	/* calculate magniture & phase for 3x3 neighbourhood */
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

	/* clean-up */
	image_free(&buff1);
	image_free(&buff2);

	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* frame_laplace(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
{
	my1frame_t buff1, buff2;
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1frame_kernel_t kernel;
	if (!frame_kernel_init(&kernel,3))
		return img;
	frame_kernel_make(&kernel,coeff);
	frame_init(&buff1);
	frame_init(&buff2);
	if(frame_make(&buff1,img->height,img->width)&&
		frame_make(&buff2,img->height,img->width))
	{
		frame_get_image(&buff1,img,1);
		frame_correlate(&buff1,&buff2,&kernel);
		if(!res->data) image_make(res,img->height,img->width);
		frame_set_image(&buff2,res,1);
	}
	frame_free(&buff1);
	frame_free(&buff2);
	frame_kernel_free(&kernel);
	return res;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_gauss(my1image_t* img, my1image_t* res,
	void* userdata, void* passdata)
{
	int coeff[] = { 1,4,7,4,1, 4,16,26,16,4,
		7,26,41,26,7, 4,16,26,16,4, 1,4,7,4,1};
	image_mask_this(img,res,5,coeff);
	image_scale(res,(1.0/273.0));
	return res;
}
/*----------------------------------------------------------------------------*/
