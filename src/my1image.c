/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#ifdef __MY1IMAGE_BASIC__
#define __MYIMAGE_NO_HSV__
#define __MYIMAGE_NO_FILE__
#define __MYIMAGE_NO_WORK__
/** #define __MYIMAGE_NO_DATA__ */
#define __MYIMAGE_NO_VIEW__
/** #define __MYIMAGE_NO_APPW__ */
/** #define __MYIMAGE_NO_DATA__ */
#endif
/*----------------------------------------------------------------------------*/
#include "my1image_base.c"
#include "my1image_gray.c"
#include "my1image_crgb.c"
#ifndef __MYIMAGE_NO_HSV__
#include "my1image_chsv.c"
#endif
#ifndef __MYIMAGE_NO_FILE__
#include "my1image_file.c"
#include "my1image_file_bmp.c"
#include "my1image_file_pnm.c"
#ifndef __MYIMAGE_NO_PNG__
#include "my1image_file_png.c"
#endif
#endif
#include "my1image_mask.c"
#include "my1image_area.c"
#include "my1image_buff.c"
#include "my1image_util.c"
#include "my1image_stat.c"
#include "my1image_scan.c"
#include "my1image_mono.c"
#ifndef __MYIMAGE_NO_WORK__
#include "my1image_work.c"
#else
#ifndef __MY1IMAGE_NO_DATA__
#define __MY1IMAGE_NO_DATA__
#endif
#endif
#ifndef __MYIMAGE_NO_VIEW__
#include "my1image_view.c"
#else
#ifndef __MY1IMAGE_NO_APPW__
#define __MY1IMAGE_NO_APPW__
#endif
#endif
#ifndef __MYIMAGE_NO_APPW__
#include "my1image_appw.c"
#else
#ifndef __MY1IMAGE_NO_DATA__
#define __MY1IMAGE_NO_DATA__
#endif
#endif
#ifndef __MY1IMAGE_NO_DATA__
#include "my1image_hist.c"
#include "my1image_data.c"
#endif
/*----------------------------------------------------------------------------*/
