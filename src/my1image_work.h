/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_WORKH__
#define __MY1IMAGE_WORKH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#define IFNAME_BINARY "binary"
#define IFNAME_BINARY_MID "binary_mid"
#define IFNAME_MORPH_ERODE "morph_erode"
#define IFNAME_MORPH_DILATE "morph_dilate"
#define IFNAME_GRAYSCALE "grayscale"
#define IFNAME_COLORBLUE "color_b2gray"
#define IFNAME_COLORGREEN "color_g2gray"
#define IFNAME_COLORRED "color_r2gray"
#define IFNAME_GRAYBLUE "gray2b"
#define IFNAME_GRAYGREEN "gray2g"
#define IFNAME_GRAYRED "gray2r"
#define IFNAME_INVERT "invert"
#define IFNAME_RESIZE "resize"
#define IFNAME_LAPLACE "laplace"
#define IFNAME_SOBELX "sobel_x"
#define IFNAME_SOBELY "sobel_y"
#define IFNAME_SOBEL "sobel"
#define IFNAME_GAUSS "gauss"
#define IFNAME_MAXSCALE "maxscale"
#define IFNAME_SUPPRESS "suppress"
#define IFNAME_THRESHOLD "threshold"
/*----------------------------------------------------------------------------*/
#define RESIZE_DEF_W 320
#define RESIZE_DEF_H 240
/*----------------------------------------------------------------------------*/
/* creates a malloc'ed my1image_filter_t instance */
my1image_filter_t* image_work_create(char* name);
my1image_filter_t* image_work_create_all(void);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_WORKH__ */
/*----------------------------------------------------------------------------*/
