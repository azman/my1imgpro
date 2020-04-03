/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_BASEH__
#define __MY1IMAGE_BASEH__
/*----------------------------------------------------------------------------*/
typedef struct _my1image_t
{
	int cols,rows;
	int size; /* memory is cheap - precalculate this! */
	int mask; /* general flag - can store rgb-encoded int! */
	int *data; /* 1-d vector for 2-d image */
}
my1image_t;
/*----------------------------------------------------------------------------*/
void image_init(my1image_t *image);
void image_free(my1image_t *image);
int* image_make(my1image_t *image, int height, int width);
void image_copy(my1image_t *dst, my1image_t *src);
void image_fill(my1image_t *image, int value);
int image_get_pixel(my1image_t *image, int row, int col); /* col(x),row(y) */
void image_set_pixel(my1image_t *image, int row, int col, int pixel);
int* image_row_data(my1image_t *image, int row);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_BASEH__ */
/*----------------------------------------------------------------------------*/
