/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGMATHH__
#define __MY1IMGMATHH__
/*----------------------------------------------------------------------------*/
#include "my1imgutil.h"
/*----------------------------------------------------------------------------*/
typedef my1Image my1Matrix; /* use the same struct - easier! */
#define creatematrix createimage
/** int* creatematrix(my1Matrix*,int height,int width); */
#define freematrix freeimage
/** void freematrix(my1Matrix*); */
#define copymatrix copyimage
/** void copymatrix(my1Matrix *src, my1Matrix *dst); */
#define image2matrix image2sub
/** void image2matrix(my1Image*,my1Matrix*,my1Region*); */
#define matrix2image sub2image
/** void matrix2image(my1Matrix*,my1Image*,my1Region*); */
#define fillmatrix fillimage
/** void fillmatrix(my1Matrix*, int value); */
#define getrow imgrowdata
/** int* getrow(my1Matrix*, int row); */
/*----------------------------------------------------------------------------*/
typedef int* my1Vector;
/*----------------------------------------------------------------------------*/
/* image operations - requires math lib! */
void image_rotate(my1Image *image, my1Image *result, int angle, int vin);
/* matrix operations */
void identitymatrix(my1Matrix *matrix); /* must be square! */
void transposematrix(my1Matrix *matrix1, my1Matrix *matrix2);
void matrix_add(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result);
void matrix_sub(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result);
void matrix_mul(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result);
void matrix_dot(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result);
void matrix_div(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
