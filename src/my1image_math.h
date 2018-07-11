/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MATHH__
#define __MY1IMAGE_MATHH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#ifndef MY1NUM_USE_DOUBLE
#define DATATYPE int
#else
#define DATATYPE double
#endif
/*----------------------------------------------------------------------------*/
typedef struct _my1num_t
{
	DATATYPE real, imag;
}
my1num_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1pol_t
{
	DATATYPE value, phase; /* polar values */
}
my1pol_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1matrix_t
{
	int width,height;
	int length; /* memory is cheap - precalculate this! */
	my1num_t *data; /* 1-d vector for 2/3-d data */
}
my1matrix_t;
/*----------------------------------------------------------------------------*/
typedef my1num_t* my1vector_t;
/*----------------------------------------------------------------------------*/
void number_get_polar(my1num_t* num, my1pol_t* pol);
void number_set_polar(my1num_t* num, my1pol_t* pol);
/*----------------------------------------------------------------------------*/
/* matrix management */
void matrix_init(my1matrix_t* mat);
void matrix_free(my1matrix_t* mat);
my1num_t* matrix_size(my1matrix_t* mat, int height, int width);
void matrix_copy(my1matrix_t* dst, my1matrix_t* src);
void matrix_fill(my1matrix_t* mat, DATATYPE real, DATATYPE imag);
my1num_t* matrix_get_row(my1matrix_t* mat, int row);
void image_get_matrix(my1image_t *img, my1matrix_t *mat, my1region_t *reg);
void image_set_matrix(my1image_t *img, my1matrix_t *mat, my1region_t *reg);
/*----------------------------------------------------------------------------*/
/* matrix operations */
void matrix_identity(my1matrix_t *mat); /* must be square! */
void matrix_transpose(my1matrix_t *mat, my1matrix_t *res);
void matrix_add(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2);
void matrix_sub(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2);
void matrix_mul(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2);
void matrix_dot(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2);
void matrix_div(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2);
/*----------------------------------------------------------------------------*/
/* here because it needs math lib! */
void image_rotate(my1image_t *image, my1image_t *check, double radian, int vin);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
