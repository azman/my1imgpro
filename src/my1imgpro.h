/*
	my1imgpro => basic image processing library for 8-bit grayscale images
	Author: Azman M. Yusof (azman@my1matrix.net)
*/

#ifndef __MY1IMGPROH__
#define __MY1IMGPROH__

#define GRAYLEVEL 256 /* 8-bit grayscale */
#define WHITE 255 /* 8-bit grayscale */
#define BLACK 0 /* 8-bit grayscale */

struct _image
{
	int width,height;
	int length; /* memory is cheap - precalculate this! */
	int mask; /* can store rgb-encoded int - use this at user level */
	int *data; /* 1-d vector for 2-d image */
};
typedef struct _image my1Image;

/* management functions */
void initimage(my1Image *image);
int* createimage(my1Image *image, int height, int width);
void freeimage(my1Image *image);
void copyimage(my1Image *src, my1Image *dst);
void fillimage(my1Image *src, int value);
int imagepixel(my1Image *image, int row, int col); /* col(x),row(y) */
void setimagepixel(my1Image *image, int row, int col, int pixel);
int* imgrowdata(my1Image *image, int row);
/* pixel operators */
void limit_pixel(my1Image *image);
void negate_pixel(my1Image *image);
void absolute_pixel(my1Image *image);
void binary_pixel(my1Image *image, int threshold);
void range_pixel(my1Image *image, int lo, int hi); /* hilite in-range pixels */
void cliphi_pixel(my1Image *image, int hithresh);
void cliplo_pixel(my1Image *image, int lothresh);
void shift_pixel(my1Image *image, int value);
void scale_pixel(my1Image *image, float value);
/* image functions */
void image_add(my1Image *image1, my1Image *image2, my1Image *result);
void image_sub(my1Image *image1, my1Image *image2, my1Image *result);
void image_pan(my1Image *image, my1Image *result, int shx, int shy, int vin);

#endif
