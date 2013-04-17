/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGBMPH__
#define __MY1IMGBMPH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
#define BMP_ERROR_FILEOPEN -1
#define BMP_ERROR_VALIDBMP -2
#define BMP_ERROR_FILESIZE -3
#define BMP_ERROR_RGBNGRAY -4
#define BMP_ERROR_MEMALLOC -5
/*----------------------------------------------------------------------------*/
typedef struct
{
	/* unsigned char bmpID[2]; / * excluded! 2 bytes */
	unsigned int bmpSize; /* 4 bytes */
	unsigned int bmpReserved; /* 4 bytes */
	unsigned int bmpOffset; /* 4 bytes */
}
my1BMPHead;
#define BMP_HEAD_SIZE 12
/*----------------------------------------------------------------------------*/
typedef struct
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
my1BMPInfo;
#define BMP_INFO_SIZE 40
/*----------------------------------------------------------------------------*/
int loadBMPimage(char *filename, my1Image *image);
int saveBMPimage(char *filename, my1Image *image);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
