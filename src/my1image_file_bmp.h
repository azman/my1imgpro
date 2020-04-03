/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILE_BMPH__
#define __MY1IMAGE_FILE_BMPH__
/*----------------------------------------------------------------------------*/
#include "my1image_file_fmt.h"
/*----------------------------------------------------------------------------*/
#define BMP_ERROR_FLAG 0x00010000
#define BMP_ERROR_MEMALLOC (BMP_ERROR_FLAG|FILE_ERROR_MALLOC)
#define BMP_ERROR_FILESIZE (BMP_ERROR_FLAG|FILE_ERROR_SIZE)
#define BMP_ERROR_FORMAT (BMP_ERROR_FLAG|FILE_ERROR_FORMAT)
#define BMP_ERROR_RGBNGRAY (BMP_ERROR_FORMAT|0x00008000)
#define BMP_ERROR_DIBINVAL (BMP_ERROR_FORMAT|0x00004000)
#define BMP_ERROR_COMPRESS (BMP_ERROR_FORMAT|0x00002000)
/*----------------------------------------------------------------------------*/
int image_load_bmp(my1image_t *image, FILE *bmpfile);
int image_save_bmp(my1image_t *image, FILE *bmpfile);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_FILE_BMPH__ */
/*----------------------------------------------------------------------------*/
