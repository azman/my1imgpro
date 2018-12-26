/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILEH__
#define __MY1IMAGE_FILEH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#define BMP_ERROR_OFFSET -10
#define BMP_ERROR_FILEOPEN -1+BMP_ERROR_OFFSET
#define BMP_ERROR_VALIDBMP -2+BMP_ERROR_OFFSET
#define BMP_ERROR_FILESIZE -3+BMP_ERROR_OFFSET
#define BMP_ERROR_RGBNGRAY -4+BMP_ERROR_OFFSET
#define BMP_ERROR_MEMALLOC -5+BMP_ERROR_OFFSET
#define BMP_ERROR_DIBINVAL -6+BMP_ERROR_OFFSET
#define BMP_ERROR_COMPRESS -7+BMP_ERROR_OFFSET
/*----------------------------------------------------------------------------*/
#define PNM_ERROR_OFFSET -20
#define PNM_ERROR_FILEOPEN -1+PNM_ERROR_OFFSET
#define PNM_ERROR_VALIDPNM -2+PNM_ERROR_OFFSET
#define PNM_ERROR_FILESIZE -3+PNM_ERROR_OFFSET
#define PNM_ERROR_NOSUPPORT -4+PNM_ERROR_OFFSET
#define PNM_ERROR_MEMALLOC -5+PNM_ERROR_OFFSET
#define PNM_ERROR_LEVELPNM -6+PNM_ERROR_OFFSET
/*----------------------------------------------------------------------------*/
#define FILE_ERROR_OPEN -1
#define FILE_ERROR_FORMAT BMP_ERROR_VALIDBMP+PNM_ERROR_VALIDPNM
/*----------------------------------------------------------------------------*/
int image_load(my1image_t *image, char *filename);
int image_save(my1image_t *image, char *filename);
int image_cdat(my1image_t *image, char *filename); /* in c-array format */
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
