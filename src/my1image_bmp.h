/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_BMPH__
#define __MY1IMAGE_BMPH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#define BMP_ERROR_FILEOPEN -1
#define BMP_ERROR_VALIDBMP -2
#define BMP_ERROR_FILESIZE -3
#define BMP_ERROR_RGBNGRAY -4
#define BMP_ERROR_MEMALLOC -5
#define BMP_ERROR_DIBINVAL -6
#define BMP_ERROR_COMPRESS -7
/*----------------------------------------------------------------------------*/
#define BMP_ID_SIZE 2
#define BMP_HEAD_SIZE 12
/*----------------------------------------------------------------------------*/
typedef struct _my1image_bmp_head_t
{
	/* unsigned char bmpID[2]; / * excluded! 2 bytes */
	unsigned int bmpSize; /* 4 bytes */
	unsigned int bmpReserved; /* 4 bytes */
	unsigned int bmpOffset; /* 4 bytes */
}
my1image_bmp_head_t;
/*----------------------------------------------------------------------------*/
#define BMP_INFO_SIZE 40
#define BMP_INFO_SIZE_V4 108
#define BMP_INFO_SIZE_V5 124
/*----------------------------------------------------------------------------*/
typedef struct _my1image_bmp_info_t
{
	unsigned int bmpInfoSize; /* 4 bytes */
	unsigned int bmpWidth; /* 4 bytes */
	unsigned int bmpHeight; /* 4 bytes */
	unsigned short bmpPlaneCount; /* 2 bytes */
	unsigned short bmpBitsPerPixel; /* 2 bytes - 1, 4, 8, 24*/
	unsigned int bmpCompression; /* 4 bytes */
	unsigned int bmpDataSize; /* 4 bytes - rounded to next 4 bytesize */
	unsigned int bmpHResolution; /* 4 bytes - pix per meter */
	unsigned int bmpVResolution; /* 4 bytes - pix per meter */
	unsigned int bmpColorCount; /* 4 bytes */
	unsigned int bmpIColorCount; /* 4 bytes */
}
my1image_bmp_info_t;
/*----------------------------------------------------------------------------*/
int image_load_bmp(char *filename, my1image_t *image);
int image_save_bmp(char *filename, my1image_t *image);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
