/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_BASEC__
#define __MY1IMAGE_BASEC__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /** for malloc and free? */
/*----------------------------------------------------------------------------*/
void image_init(my1image_t *image)
{
	image->cols = 0;
	image->rows = 0;
	image->size = 0;
	image->mask = 0;
	image->data = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_free(my1image_t *image)
{
	if (image->data) free((void*)image->data);
	image->data = 0x0;
	image->size = 0;
}
/*----------------------------------------------------------------------------*/
int* image_make(my1image_t *image, int rows, int cols)
{
	int size = rows*cols;
	int *temp = image->data;
	if (!temp||image->rows!=rows||image->cols!=cols)
	{
		temp = (int*) realloc(image->data,size*sizeof(int));
		if (temp)
		{
			image->data = temp;
			image->rows = rows;
			image->cols = cols;
			image->size = size;
		}
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void image_copy(my1image_t *dst, my1image_t *src)
{
	int loop;
	image_make(dst,src->rows,src->cols);
	for (loop=0;loop<dst->size;loop++)
		dst->data[loop] = src->data[loop];
	dst->mask = src->mask;
}
/*----------------------------------------------------------------------------*/
void image_fill(my1image_t *image, int value)
{
	int loop, size = image->size;
	for (loop=0;loop<size;loop++)
		image->data[loop] = value;
}
/*----------------------------------------------------------------------------*/
int image_get_pixel(my1image_t *image, int row, int col) /* col(x),row(y) */
{
	return image->data[row*image->cols+col];
}
/*----------------------------------------------------------------------------*/
void image_set_pixel(my1image_t *image, int row, int col, int pixel)
{
	image->data[row*image->cols+col] = pixel;
}
/*----------------------------------------------------------------------------*/
int* image_row_data(my1image_t *image, int row)
{
	return &image->data[row*image->cols];
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_BASEC__ */
/*----------------------------------------------------------------------------*/
