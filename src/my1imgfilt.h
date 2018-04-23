/*----------------------------------------------------------------------------*/
#include "my1imgutil.h"
#include "my1imgfpo.h"
/*----------------------------------------------------------------------------*/
my1ImgFilter* insert_imgfilter(my1ImgFilter* pstack, my1ImgFilter* pcheck);
my1Image* apply_mask2image(my1Image* image, my1Image* result,
	int size, int* pdata);
my1Image* laplace_image(my1Image* image, my1Image* result, void* userdata);
my1Image* sobel_x_image(my1Image* image, my1Image* result, void* userdata);
my1Image* sobel_y_image(my1Image* image, my1Image* result, void* userdata);
my1Image* sobel_image(my1Image* image, my1Image* result, void* userdata);
my1Image* laplace_frame(my1Image* image, my1Image* result, void* userdata);
my1Image* gaussian_image(my1Image* image, my1Image* result, void* userdata);
/*----------------------------------------------------------------------------*/
