/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGPNMH__
#define __MY1IMGPNMH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
#define PNM_ERROR_FILEOPEN -1
#define PNM_ERROR_VALIDPNM -2
#define PNM_ERROR_FILESIZE -3
#define PNM_ERROR_NOSUPPORT -4
#define PNM_ERROR_MEMALLOC -5
/*----------------------------------------------------------------------------*/
int image_load_pnm(char *filename, my1image_t *image);
int image_save_pnm(char *filename, my1image_t *image);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
