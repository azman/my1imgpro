/*----------------------------------------------------------------------------*/
#include "my1image_math.h"
/*----------------------------------------------------------------------------*/
#include <math.h> /* trig functions */
#include <stdlib.h> /* malloc and free! */
/*----------------------------------------------------------------------------*/
void number_get_polar(my1num_t* num, my1pol_t* pol)
{
	pol->value = sqrt((num->real*num->real)+(num->imag*num->imag));
	if (num->real==(DATATYPE)0) pol->phase = (DATATYPE)0;
	else pol->phase = (DATATYPE) atan((double)num->imag/num->real);
}
/*----------------------------------------------------------------------------*/
void number_set_polar(my1num_t* num, my1pol_t* pol)
{
	num->real = (DATATYPE)(pol->value*cos((double)pol->phase));
	num->imag = (DATATYPE)(pol->value*sin((double)pol->phase));
}
/*----------------------------------------------------------------------------*/
void number_multiply(my1num_t* dst, my1num_t* src1, my1num_t* src2)
{
	dst->real = (DATATYPE)((src1->real*src2->real)-(src1->imag*src2->imag));
	dst->imag = (DATATYPE)((src1->real*src2->imag)+(src1->imag*src2->real));
}
/*----------------------------------------------------------------------------*/
void number_divide(my1num_t* dst, my1num_t* src1, my1num_t* src2)
{
	/* get denominator */
	DATATYPE temp = (DATATYPE)(src2->real*src2->real)+(src2->imag*src2->imag);
	/* temp should not be zero! */
	if (temp!=(DATATYPE)0)
	{
		dst->real = (DATATYPE)(((src1->real*src2->real)+
			(src1->imag*src2->imag))/temp);
		dst->imag = (DATATYPE)(((src1->imag*src2->real)+
			(src1->real*src2->imag))/temp);
	}
	else
	{
		dst->real = (DATATYPE)0;
		dst->imag = (DATATYPE)0;
	}
}
/*----------------------------------------------------------------------------*/
void matrix_init(my1matrix_t* mat)
{
	mat->width = 0;
	mat->height = 0;
	mat->length = 0;
	mat->data = 0x0;
}
/*----------------------------------------------------------------------------*/
void matrix_free(my1matrix_t* mat)
{
	if (mat->data) free((void*)mat->data);
	mat->data = 0x0;
	mat->length = 0;
}
/*----------------------------------------------------------------------------*/
my1num_t* matrix_size(my1matrix_t* mat, int height, int width)
{
	int irow, icol;
	int length = height*width;
	my1num_t *temp = (my1num_t*) realloc(mat->data,length*sizeof(my1num_t));
	if (temp)
	{
		/* copy data to new locations if possible */
		if (mat->length>0&&mat->length<length)
		{
			for (irow=height-1;irow>0;irow--)
			{
				for (icol=width-1;icol>0;icol--)
				{
					int old = irow*mat->width+icol;
					if (old<mat->length)
					{
						int new = irow*width+icol;
						mat->data[new] = mat->data[old];
					}
				}
			}
		}
		mat->width = width;
		mat->height = height;
		mat->length = length;
		mat->data = temp;
	}
	return temp;
}
/*----------------------------------------------------------------------------*/
void matrix_copy(my1matrix_t* dst, my1matrix_t* src)
{
	int icol;
	if (dst->length!=src->length)
		matrix_size(dst,src->height,src->width);
	/** just in case... */
	dst->width = src->width;
	dst->height = src->height;
	/** copy values */
	for (icol=0;icol<dst->length;icol++)
		dst->data[icol] = src->data[icol];
}
/*----------------------------------------------------------------------------*/
void matrix_fill(my1matrix_t* mat, DATATYPE real, DATATYPE imag)
{
	int loop;
	for (loop=0;loop<mat->length;loop++)
	{
		mat->data[loop].real = real;
		mat->data[loop].imag = imag;
	}
}
/*----------------------------------------------------------------------------*/
my1num_t* matrix_get_row(my1matrix_t* mat, int row)
{
	return (my1num_t*) &(mat->data[row*mat->width]);
}
/*----------------------------------------------------------------------------*/
void image_get_matrix(my1image_t *img,my1matrix_t *mat,my1image_region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		xoff = reg->xset;
		yoff = reg->yset;
		row = reg->height;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImg = image_row_data(img,iloop+yoff);
		my1num_t* pMat = matrix_get_row(mat,iloop);
		for (jloop=0;jloop<col;jloop++)
		{
			pMat[jloop].real = (DATATYPE) pImg[jloop+xoff];
			pMat[jloop].imag = (DATATYPE) 0;
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_set_matrix(my1image_t *img,my1matrix_t *mat,my1image_region_t *reg)
{
	int iloop, jloop, xoff = 0, yoff = 0;
	int row = img->height, col = img->width;
	if (reg)
	{
		xoff = reg->xset;
		yoff = reg->yset;
		row = reg->height;
		col = reg->width;
	}
	for (iloop=0;iloop<row;iloop++)
	{
		int* pImg = image_row_data(img,iloop+yoff);
		my1num_t* pMat = matrix_get_row(mat,iloop);
		for (jloop=0;jloop<col;jloop++)
		{
			int temp = (int) pMat[jloop].real;
			if (pMat[jloop].imag!=(DATATYPE)0)
			{
				my1pol_t buff;
				number_get_polar(&pMat[jloop],&buff);
				temp = (int) buff.value;
			}
			pImg[jloop+xoff] = temp;
		}
	}
}
/*----------------------------------------------------------------------------*/
void matrix_identity(my1matrix_t *mat)
{
	int loop, size = mat->width;
	/* use lower-sized dimension for non-square matrix! */
	if(mat->height<mat->width) size = mat->height;
	/* clear everything first! */
	matrix_fill(mat,(DATATYPE)0,(DATATYPE)0);
	/* create ones on main diagonal */
	for(loop=0;loop<size;loop++)
		matrix_get_row(mat,loop)[loop].real = (DATATYPE)1;
}
/*----------------------------------------------------------------------------*/
void matrix_transpose(my1matrix_t *mat, my1matrix_t *res)
{
	int iloop, jloop;
	int row = mat->height, col = mat->width;
	for(iloop=0;iloop<col;iloop++)
	{
		my1vector_t pvec = matrix_get_row(res,iloop);
		for(jloop=0;jloop<row;jloop++)
		{
			pvec[jloop] = matrix_get_row(mat,jloop)[iloop];
		}
	}
}
/*----------------------------------------------------------------------------*/
void matrix_add(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2)
{
	int iloop, ilength = ma1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		res->data[iloop].real = ma1->data[iloop].real + ma2->data[iloop].real;
		res->data[iloop].imag = ma1->data[iloop].imag + ma2->data[iloop].imag;
	}
}
/*----------------------------------------------------------------------------*/
void matrix_sub(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2)
{
	int iloop, ilength = ma1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		res->data[iloop].real = ma1->data[iloop].real - ma2->data[iloop].real;
		res->data[iloop].imag = ma1->data[iloop].imag - ma2->data[iloop].imag;
	}
}
/*----------------------------------------------------------------------------*/
void matrix_mul(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2)
{
	int iloop, jloop, kloop;
	int row = ma1->height, col = ma2->width, idx = ma1->width;
	my1num_t temp;
	for(iloop=0;iloop<row;iloop++)
	{
		my1vector_t p1 = matrix_get_row(ma1,iloop);
		my1vector_t pr = matrix_get_row(res,iloop);
		for(jloop=0;jloop<col;jloop++)
		{
			pr[jloop].real = (DATATYPE)0;
			pr[jloop].imag = (DATATYPE)0;
			for(kloop=0;kloop<idx;kloop++)
			{
				number_multiply(&temp,&p1[kloop],
					&matrix_get_row(ma2,kloop)[jloop]);
				pr[jloop].real += temp.real;
				pr[jloop].imag += temp.imag;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void matrix_dot(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2)
{
	int iloop, ilength = ma1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		number_multiply(&res->data[iloop],&ma1->data[iloop],&ma2->data[iloop]);
	}
}
/*----------------------------------------------------------------------------*/
void matrix_div(my1matrix_t *res, my1matrix_t *ma1, my1matrix_t *ma2)
{
	int iloop, ilength = ma1->length;
	for(iloop=0;iloop<ilength;iloop++)
	{
		number_divide(&res->data[iloop],&ma1->data[iloop],&ma2->data[iloop]);
	}
}
/*----------------------------------------------------------------------------*/
void image_rotate(my1image_t *image, my1image_t *check, int radian, int vin)
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
				calcv = image_get_pixel(image,calcy,calcx);
				image_set_pixel(check,iloop,jloop,calcv);
			}
			else
			{
				image_set_pixel(check,iloop,jloop,vin);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
