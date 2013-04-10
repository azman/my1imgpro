/*----------------------------------------------------------------------------*/
#include "my1imgmath.h"
#include <math.h> /* trig functions */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void image_rotate(my1Image *image, my1Image *result, int radian, int vin)
{
	int iloop, jloop;
	int calcx, calcy, calcv;
	int row = image->height, col = image->width;
	int midx = col/2, midy = row/2;
	int tempx, tempy; /* needs to be float to avoid round error */
	/* calculate new pixel index */
	for(iloop=0;iloop<row;iloop++)
	{
		for(jloop=0;jloop<col;jloop++)
		{
			/* get virtual coord - origin at image center */
			calcx = jloop - midx;
			calcy = (col-iloop) - midy;
			/* get virtual rotated index */
			tempx = (calcx*cos(radian))+(calcy*sin(radian));
			tempy = (calcy*cos(radian))-(calcx*sin(radian));
			/* calculate actual coord */
			calcx = tempx + midx;
			calcy = col - (tempy+midy);
			if(calcx>=0&&calcx<col&&calcy>=0&&calcy<row)
			{
				calcv = imagepixel(image,calcy,calcx);
				setimagepixel(result,iloop,jloop,calcv);
			}
			else
			{
				setimagepixel(result,iloop,jloop,vin);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void identitymatrix(my1Matrix *matrix)
{
	int iloop;
	/* ignore if NOT square matrix! */
	if(matrix->width!=matrix->height)
		return;
	/* clear everything first! */
	for(iloop=0;iloop<matrix->length;iloop++)
		matrix->data[iloop] = 0;
	/* create ones on main diagonal */
	for(iloop=0;iloop<matrix->height;iloop++)
		getrow(matrix,iloop)[iloop] = 1;
}
/*----------------------------------------------------------------------------*/
void transposematrix(my1Matrix *matrix1, my1Matrix *matrix2)
{
	int iloop, jloop;
	int row = matrix1->height, col = matrix1->width;
	for(iloop=0;iloop<col;iloop++)
	{
		my1Vector pMatrix2 = getrow(matrix2,iloop);
		for(jloop=0;jloop<row;jloop++)
		{
			pMatrix2[jloop] = getrow(matrix1,jloop)[iloop];
		}
	}
}
/*----------------------------------------------------------------------------*/
void matrix_add(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result)
{
	int iloop, ilength = matrix1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		result->data[iloop] =
			matrix1->data[iloop] + matrix2->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void matrix_sub(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result)
{
	int iloop, ilength = matrix1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		result->data[iloop] =
			matrix1->data[iloop] - matrix2->data[iloop];
	}
}
/*----------------------------------------------------------------------------*/
void matrix_mul(my1Matrix *matrix1, my1Matrix *matrix2, my1Matrix *result)
{
	int iloop, jloop, kloop;
	int row = matrix1->height, col = matrix2->width, idx = matrix1->width;
	for(iloop=0;iloop<row;iloop++)
	{
		my1Vector pMatrix1 = getrow(matrix1,iloop);
		my1Vector pMatrixr = getrow(result,iloop);
		for(jloop=0;jloop<col;jloop++)
		{
			pMatrixr[jloop] = 0;
			for(kloop=0;kloop<idx;kloop++)
			{
				pMatrixr[jloop] +=
					pMatrix1[kloop] * getrow(matrix2,kloop)[jloop];
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
