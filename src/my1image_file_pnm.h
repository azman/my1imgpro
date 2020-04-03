/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILE_PNMH__
#define __MY1IMAGE_FILE_PNMH__
/*----------------------------------------------------------------------------*/
#include "my1image_file_fmt.h"
/*----------------------------------------------------------------------------*/
#define PNM_ERROR_FLAG 0x00020000
#define PNM_ERROR_MEMALLOC (PNM_ERROR_FLAG|FILE_ERROR_MALLOC)
#define PNM_ERROR_FILESIZE (PNM_ERROR_FLAG|FILE_ERROR_SIZE)
#define PNM_ERROR_FORMAT (PNM_ERROR_FLAG|FILE_ERROR_FORMAT)
#define PNM_ERROR_NOSUPPORT (PNM_ERROR_FORMAT|0x00008000)
#define PNM_ERROR_LEVELPNM (PNM_ERROR_FORMAT|0x00004000)
#define PNM_ERROR_CORRUPT (PNM_ERROR_FORMAT|0x00002000)
/*----------------------------------------------------------------------------*/
int image_load_pnm(my1image_t *image, FILE *pnmfile);
int image_save_pnm(my1image_t *image, FILE *pnmfile);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_FILE_PNMH__ */
/*----------------------------------------------------------------------------*/
