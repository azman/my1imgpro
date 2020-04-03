/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILE_PNGH__
#define __MY1IMAGE_FILE_PNGH__
/*----------------------------------------------------------------------------*/
#include "my1image_file_fmt.h"
/*----------------------------------------------------------------------------*/
#define PNG_ERROR_FLAG 0x00040000
#define PNG_ERROR_MEMALLOC (PNG_ERROR_FLAG|FILE_ERROR_MALLOC)
#define PNG_ERROR_FORMAT (PNG_ERROR_FLAG|FILE_ERROR_FORMAT)
#define PNG_ERROR_NOHEADER (PNG_ERROR_FORMAT|0x00008000)
#define PNG_ERROR_NOSUPPORT (PNG_ERROR_FORMAT|0x00004000)
#define PNG_ERROR_MISSCHUNK (PNG_ERROR_FORMAT|0x00002000)
#define PNG_ERROR_PALLETTE (PNG_ERROR_FORMAT|0x00001000)
#define PNG_ERROR_ZLIBCOMP (PNG_ERROR_FORMAT|0x00000800)
#define PNG_ERROR_DATACHUNK (PNG_ERROR_FORMAT|0x00000400)
/*----------------------------------------------------------------------------*/
extern my1imgfmt_t ipng;
/*----------------------------------------------------------------------------*/
int image_load_png(my1image_t *image, FILE *pngfile);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_FILE_PNGH__ */
/*----------------------------------------------------------------------------*/
