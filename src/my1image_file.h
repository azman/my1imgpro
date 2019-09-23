/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILEH__
#define __MY1IMAGE_FILEH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
/** a fancy way of setting msb to 1 */
#define FILE_ERROR_FLAG (~(~0U>>1))
#define FILE_ERROR_OPEN (0x00000001|FILE_ERROR_FLAG)
#define FILE_ERROR_FORMAT (0x00000002|FILE_ERROR_FLAG)
#define FILE_ERROR_SIZE (0x00000004|FILE_ERROR_FLAG)
#define FILE_ERROR_MALLOC (0x00000008|FILE_ERROR_FLAG)
/*----------------------------------------------------------------------------*/
#define BMP_ERROR_FLAG 0x00010000
#define BMP_ERROR_FILEOPEN (BMP_ERROR_FLAG|FILE_ERROR_OPEN)
#define BMP_ERROR_VALIDBMP (BMP_ERROR_FLAG|FILE_ERROR_FORMAT)
#define BMP_ERROR_FILESIZE (BMP_ERROR_FLAG|FILE_ERROR_SIZE)
#define BMP_ERROR_MEMALLOC (BMP_ERROR_FLAG|FILE_ERROR_MALLOC)
#define BMP_ERROR_RGBNGRAY (BMP_ERROR_FLAG|0x00008000)
#define BMP_ERROR_DIBINVAL (BMP_ERROR_FLAG|0x00004000)
#define BMP_ERROR_COMPRESS (BMP_ERROR_FLAG|0x00002000)
/*----------------------------------------------------------------------------*/
#define PNM_ERROR_FLAG 0x00020000
#define PNM_ERROR_FILEOPEN (PNM_ERROR_FLAG|FILE_ERROR_OPEN)
#define PNM_ERROR_VALIDPNM (PNM_ERROR_FLAG|FILE_ERROR_FORMAT)
#define PNM_ERROR_FILESIZE (PNM_ERROR_FLAG|FILE_ERROR_SIZE)
#define PNM_ERROR_MEMALLOC (PNM_ERROR_FLAG|FILE_ERROR_MALLOC)
#define PNM_ERROR_NOSUPPORT (PNM_ERROR_FLAG|0x00008000)
#define PNM_ERROR_LEVELPNM (PNM_ERROR_FLAG|0x00004000)
/*----------------------------------------------------------------------------*/
int image_load(my1image_t *image, char *filename);
int image_save(my1image_t *image, char *filename);
int image_cdat(my1image_t *image, char *filename); /* in c-array format */
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
