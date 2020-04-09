/*----------------------------------------------------------------------------*/
/** USEFUL PRESETS */
#if defined(__MY1IMAGE_FILE_ONLY__)
#define __MY1IMAGE_NO_HSV__
#define __MY1IMAGE_NO_UTIL__
/** #define __MYIMAGE_NO_WORK__ */
/** #define __MYIMAGE_NO_DATA__ */
#define __MY1IMAGE_NO_VIEW__
/** #define __MYIMAGE_NO_APPW__ */
/** #define __MYIMAGE_NO_DATA__ */
#elif defined(__MY1IMAGE_BASIC__)
#define __MY1IMAGE_NO_HSV__
#define __MY1IMAGE_NO_FILE__
#define __MY1IMAGE_NO_WORK__
/** #define __MYIMAGE_NO_DATA__ */
#define __MY1IMAGE_NO_VIEW__
/** #define __MYIMAGE_NO_APPW__ */
/** #define __MYIMAGE_NO_DATA__ */
#endif
/*----------------------------------------------------------------------------*/
#include "my1image_base.c"
#include "my1image_gray.c"
#include "my1image_crgb.c"
#ifndef __MY1IMAGE_NO_HSV__
#include "my1image_chsv.c"
#endif
#ifndef __MYIMAGE_NO_FILE__
#ifndef __MY1IMAGE_NO_PNG__
#ifndef __MY1IMAGE_AUTO_PNG__
#define __MY1IMAGE_AUTO_PNG__
#endif
#endif
#include "my1image_file.c"
#include "my1image_file_bmp.c"
#include "my1image_file_pnm.c"
#ifndef __MY1IMAGE_NO_PNG__
#include "my1image_file_png.c"
#endif
#endif
#ifndef __MYIMAGE_NO_UTIL__
#include "my1image_mask.c"
#include "my1image_area.c"
#include "my1image_buff.c"
#include "my1image_util.c"
#include "my1image_stat.c"
#include "my1image_scan.c"
#include "my1image_mono.c"
#else
#ifndef __MY1IMAGE_NO_WORK__
#define __MY1IMAGE_NO_WORK__
#endif
#endif
#ifndef __MY1IMAGE_NO_WORK__
#include "my1image_work.c"
#else
#ifndef __MY1IMAGE_NO_DATA__
#define __MY1IMAGE_NO_DATA__
#endif
#endif
#ifndef __MY1IMAGE_NO_VIEW__
#include "my1image_view.c"
#else
#ifndef __MY1IMAGE_NO_APPW__
#define __MY1IMAGE_NO_APPW__
#endif
#endif
#ifndef __MY1IMAGE_NO_APPW__
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
