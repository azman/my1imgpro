/*----------------------------------------------------------------------------*/
#include "my1image.h"
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
#ifdef __MYIMAGE_FILE_PNG__
#include "my1image_file_png.c"
#endif
#endif
#include "my1image_mask.c"
#include "my1image_area.c"
#include "my1image_buff.c"
#include "my1image_util.c"
#include "my1image_stat.c"
#ifdef __MYIMAGE_WORK__
#include "my1image_work.c"
#endif
#ifdef __MYIMAGE_VIEW__
#include "my1image_view.c"
#endif
#ifdef __MYIMAGE_APPW__
#include "my1image_appw.c"
#include "my1image_hist.c"
#include "my1image_data.c"
#endif
#include "my1image_scan.c"
#include "my1image_mono.c"
/*----------------------------------------------------------------------------*/
