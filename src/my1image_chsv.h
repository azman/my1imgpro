/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_CHSVH__
#define __MY1IMAGE_CHSVH__
/*----------------------------------------------------------------------------*/
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
/* image color (hsv) stuffs */
/*----------------------------------------------------------------------------*/
#define IMASK_COLOR_H 0xFFFF0000
#define IMASK_COLOR_S 0x0000FF00
#define IMASK_COLOR_V 0x000000FF
/*----------------------------------------------------------------------------*/
#define HUE_FULL 360
#define HUE_PART 6
#define HUE_DIFF (HUE_FULL/HUE_PART)
#define HUE_COMP (HUE_DIFF<<1)
#define HUE_INIT_B (HUE_FULL-HUE_COMP)
#define HUE_INIT_G (HUE_INIT_B-HUE_COMP)
#define HUE_INIT_R (HUE_INIT_G-HUE_COMP)
/*----------------------------------------------------------------------------*/
typedef unsigned short hword; /** hue word */
/*----------------------------------------------------------------------------*/
typedef struct
{
/**
 * pure integer implementation
 * - still 32-bit int representation
 * - can reproduce rgb quite well
 * - max, min is easily obtained
 * - third value is not more than (5/255)x100% error margin
 * - increasing HUE_FULL to 720 reduces that to (3/255)
**/
	cbyte v,s;
	hword h;
}
my1image_hsv_t;
/*----------------------------------------------------------------------------*/
typedef my1image_hsv_t my1hsv_t;
/*----------------------------------------------------------------------------*/
my1hsv_t rgb2hsv(my1rgb_t rgb);
my1rgb_t hsv2rgb(my1hsv_t hsv); /* not really needed? */
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_CHSVH__ */
/*----------------------------------------------------------------------------*/
