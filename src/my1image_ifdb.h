/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_IFDBH__
#define __MY1IMAGE_IFDBH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
#include "my1image_stat.h"
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
#define BINARY_CUT 100
/*----------------------------------------------------------------------------*/
/* for filter_binary */
typedef struct _my1if_options_t
{
	void* data;
	int flag, temp;
	int par1, par2;
}
my1if_options_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1if_resize_t
{
	my1image_t buff;
	my1image_area_t area;
}
my1if_resize_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1if_threshold_t
{
	my1image_histogram_t hist;
}
my1if_threshold_t;
/*----------------------------------------------------------------------------*/
/**
cat src/my1image_ifdb.c | grep -A1 "my1image_t\* filter_" | grep -v "{" | \
	grep -v '^--'
** ONLY USE THESE IF YOU KNOW WHAT YOU ARE DOING! SOME REQUIRE FILTER OBJ!
**/
my1image_t* filter_binary(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_binary_mid(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_morph_erode(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_morph_dilate(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_gray(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_color_blue(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_color_green(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_color_red(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_gray2b(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_gray2g(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_gray2r(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_invert(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_resize(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_laplace(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_sobel_x(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_sobel_y(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_sobel(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_gauss(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_maxscale(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_suppress(my1image_t*,my1image_t*,my1ipass_t*);
my1image_t* filter_threshold(my1image_t*,my1image_t*,my1ipass_t*);
/*----------------------------------------------------------------------------*/
/* find predefined filter info */
filter_info_t* image_work_find_info(char* name);
/*----------------------------------------------------------------------------*/
/* creates a malloc'ed my1image_filter_t instance */
my1ipass_t* image_work_create(char* name);
my1ipass_t* image_work_create_all(void);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_IFDBH__ */
/*----------------------------------------------------------------------------*/
