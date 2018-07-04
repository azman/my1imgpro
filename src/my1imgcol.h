/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGCOLH__
#define __MY1IMGCOLH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
#define IMASK_COLOR24 0xFFFFFF
#define IMASK_BINARY1 0x01
#define IMASK_GRAY 0x00
#define IMASK_NONE IMASK_GRAY
/*----------------------------------------------------------------------------*/
typedef unsigned char cbyte; /** color byte */
/*----------------------------------------------------------------------------*/
/* color information - structure member 'mask' MUST BE assigned! */
int image_assign_rgb(my1image_t *image, cbyte *rgb);
int image_extract_rgb(my1image_t *image, cbyte *rgb);
/* enforce grayscale image */
void image_grayscale(my1image_t *image);
/* rgb conversion utility */
int encode_rgb(cbyte r, cbyte g, cbyte b);
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
