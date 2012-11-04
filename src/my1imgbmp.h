/*
	my1imgbmp => bmp file utility for my1imgpro
	Author: azman@my1matrix.net
*/

#ifndef __MY1IMGBMPH__
#define __MY1IMGBMPH__

#include "my1imgpro.h"

#define BMP_HEADER_SIZE 54 /* bytes including "BM" */

#define BMP_ERROR_FILEOPEN -1
#define BMP_ERROR_VALIDBMP -2
#define BMP_ERROR_FILESIZE -3
#define BMP_ERROR_RGBNGRAY -4
#define BMP_ERROR_MEMALLOC -5

typedef struct
{
	unsigned int bmpSize; /* 4 bytes */
	unsigned int bmpReserved; /* 4 bytes */
	unsigned int bmpOffset; /* 4 bytes */
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

int loadBMPimage(char *filename, my1Image *image);
int saveBMPimage(char *filename, my1Image *image);

#endif
