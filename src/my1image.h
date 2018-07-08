/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGEH__
#define __MY1IMAGEH__
/*----------------------------------------------------------------------------*/
#define GRAYLEVEL 256 /* 8-bit grayscale */
#define WHITE 255 /* 8-bit grayscale */
#define BLACK 0 /* 8-bit grayscale */
/*----------------------------------------------------------------------------*/
typedef struct _my1image_t
{
	int width,height;
	int length; /* memory is cheap - precalculate this! */
	int mask; /* can store rgb-encoded int - use this at user level */
	int *data; /* 1-d vector for 2-d image */
}
my1image_t;
/*----------------------------------------------------------------------------*/
/* management functions */
void image_init(my1image_t *image);
int* image_make(my1image_t *image, int height, int width);
void image_free(my1image_t *image);
void image_copy(my1image_t *dst, my1image_t *src);
void image_fill(my1image_t *image, int value);
int image_get_pixel(my1image_t *image, int row, int col); /* col(x),row(y) */
void image_set_pixel(my1image_t *image, int row, int col, int pixel);
int* image_row_data(my1image_t *image, int row);
/* pixel operators */
void image_limit(my1image_t *image);
void image_invert(my1image_t *image);
void image_absolute(my1image_t *image);
void image_binary(my1image_t *image, int threshold);
void image_range(my1image_t *image, int lothresh, int hithresh);
void image_cliphi(my1image_t *image, int hithresh);
void image_cliplo(my1image_t *image, int lothresh);
void image_shift(my1image_t *image, int value);
void image_scale(my1image_t *image, float value);
void image_normalize(my1image_t *image); /* get max value, rescale */
/* image functions */
void image_add(my1image_t *image1, my1image_t *image2, my1image_t *result);
void image_sub(my1image_t *image1, my1image_t *image2, my1image_t *result);
void image_pan(my1image_t *image, my1image_t *result, int shx, int shy, int vin);
/*----------------------------------------------------------------------------*/
#define IMASK_COLOR24 0x00FFFFFF
#define IMASK_GRAY 0x00
/*----------------------------------------------------------------------------*/
typedef unsigned char cbyte; /** color byte */
/*----------------------------------------------------------------------------*/
/* color information - structure member 'mask' MUST BE assigned! */
int image_assign_rgb(my1image_t *image, cbyte *rgb);
int image_extract_rgb(my1image_t *image, cbyte *rgb);
/* enforce color modes */
void image_grayscale(my1image_t *image);
void image_colormode(my1image_t *image);
/* rgb conversion utility */
int encode_rgb(cbyte r, cbyte g, cbyte b);
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
