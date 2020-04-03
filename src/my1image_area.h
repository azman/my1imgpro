/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_AREAH__
#define __MY1IMAGE_AREAH__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
typedef struct _my1image_area_t
{
	int xset, yset;
	int wval, hval;
}
my1image_area_t;
/*----------------------------------------------------------------------------*/
void image_area_make(my1image_area_t *reg, int y, int x, int h, int w);
void image_area_select(my1image_t *img, my1image_area_t *reg, int val, int inv);
void image_size_aspect(my1image_t *img, my1image_area_t *reg);
/*----------------------------------------------------------------------------*/
void image_get_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
void image_set_area(my1image_t *img, my1image_t *sub, my1image_area_t *reg);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_AREAH__ */
/*----------------------------------------------------------------------------*/
