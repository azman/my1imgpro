/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MONOH__
#define __MY1IMAGE_MONOH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
/**
 * structuring element is always 3x3 with center origin
 * - array size 9... origin at index 4
 * -  [0] [1] [2]
 * -  [3] [4] [5]
 * -  [6] [7] [8]
**/
/*----------------------------------------------------------------------------*/
void image_erode(my1image_t *img, my1image_t *res, int* elem);
void image_dilate(my1image_t *img, my1image_t *res, int* elem);
void image_opening(my1image_t *img, my1image_t *res, int* elem);
void image_closing(my1image_t *img, my1image_t *res, int* elem);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
