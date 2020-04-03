/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILEH__
#define __MY1IMAGE_FILEH__
/*----------------------------------------------------------------------------*/
#include "my1image_file_fmt.h"
/*----------------------------------------------------------------------------*/
void image_format_insert(my1image_format_t* imgfmt);
/*----------------------------------------------------------------------------*/
int image_load(my1image_t *image, char *filename);
int image_save(my1image_t *image, char *filename);
int image_cdat(my1image_t *image, char *filename); /* in c-array format */
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_FILEH__ */
/*----------------------------------------------------------------------------*/
