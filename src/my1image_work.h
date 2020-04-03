/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_WORKH__
#define __MY1IMAGE_WORKH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#define IFNAME_GRAYSCALE "grayscale"
#define IFNAME_COLORBLUE "colorb"
#define IFNAME_COLORGREEN "colorg"
#define IFNAME_COLORRED "colorr"
#define IFNAME_INVERT "invert"
#define IFNAME_RESIZE "resize"
#define IFNAME_LAPLACE "laplace"
#define IFNAME_SOBELX "sobelx"
#define IFNAME_SOBELY "sobely"
#define IFNAME_SOBEL "sobel"
#define IFNAME_GAUSS "gauss"
#define IFNAME_MAXSCALE "maxscale"
#define IFNAME_SUPPRESS "suppress"
#define IFNAME_THRESHOLD "threshold"
/*----------------------------------------------------------------------------*/
/* creates a malloc'ed my1image_filter_t instance */
my1image_filter_t* image_work_create(char* name);
my1image_filter_t* image_work_create_all(void);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_WORKH__ */
/*----------------------------------------------------------------------------*/
