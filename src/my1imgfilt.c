/*----------------------------------------------------------------------------*/
#include "my1imgfilt.h"
/*----------------------------------------------------------------------------*/
#include <math.h>
/*----------------------------------------------------------------------------*/
my1Image* apply_mask2image(my1Image* image, my1Image* result,
	int size, int* pdata)
{
	my1Mask mask;
	if(!createmask(&mask,size)) return image;
	setmask(&mask,pdata);
	if(!result->data) createimage(result,image->height,image->width);
	mask_image(&mask,image,result);
	freemask(&mask);
	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* laplace_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_x_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_y_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { -1,-2,-1, 0,0,0, 1,2,1 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
int iabs(int value)
{
	if(value<0) value = -value;
	return value;
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_image(my1Image* image, my1Image* result, void* userdata)
{
	my1Image buff1, buff2, *pphase = (my1Image*) userdata;
	int irow, icol, x, y;

	/* initialize buffer stuctures */
	initimage(&buff1);
	initimage(&buff2);
	/* create temporary buffers */
	if(!createimage(&buff1,image->height,image->width)||
		!createimage(&buff2,image->height,image->width))
	{
		freeimage(&buff1);
		freeimage(&buff2);
		return image;
	}

	/* calculate directional edge */
	sobel_x_image(image, &buff1, 0x0);
	sobel_y_image(image, &buff2, 0x0);

	/* prepare resulting image structure */
	if(!result->data) createimage(result,image->height,image->width);
	if(pphase&&!pphase->data) createimage(pphase,image->height,image->width);

	/* calculate magniture & phase for 3x3 neighbourhood */
	for(irow=0;irow<image->height;irow++)
	{
		for(icol=0;icol<image->width;icol++)
		{
			x = imagepixel(&buff1,irow,icol);
			y = imagepixel(&buff2,irow,icol);
			setimagepixel(result,irow,icol,(int)sqrt((y*y)+(x*x)));
			if(!pphase) continue;
			setimagepixel(pphase,irow,icol,(int)atan2(y,x));
		}
	}

	/* clean-up */
	freeimage(&buff1);
	freeimage(&buff2);

	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* laplace_frame(my1Image* image, my1Image* result, void* userdata)
{
	my1IFrame buff1, buff2;
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1Kernel kernel;
	if(createkernel(&kernel,3))
	{
		setkernel(&kernel,coeff);
		initframe(&buff1);
		initframe(&buff2);
		if(createframe(&buff1,image->height,image->width)&&
			createframe(&buff2,image->height,image->width))
		{
			image2frame(image,&buff1,1);
			correlate_frame(&buff1,&buff2,&kernel);
			if(!result->data)
				createimage(result,image->height,image->width);
			frame2image(&buff2,result,1);
		}
		freeframe(&buff1);
		freeframe(&buff2);
		freekernel(&kernel);
	}
	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* gaussian_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { 1,4,7,4,1, 4,16,26,16,4,
		7,26,41,26,7, 4,16,26,16,4, 1,4,7,4,1};
	apply_mask2image(image,result,5,coeff);
	scale_pixel(result,(1.0/273.0));
	return result;
}
/*----------------------------------------------------------------------------*/
