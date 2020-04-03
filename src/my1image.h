/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGEH__
#define __MY1IMAGEH__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
#include "my1image_gray.h"
#include "my1image_crgb.h"
#ifndef __MYIMAGE_NO_HSV__
#include "my1image_chsv.h"
#endif
#ifndef __MYIMAGE_NO_FILE__
#include "my1image_file.h"
#include "my1image_file_bmp.h"
#include "my1image_file_pnm.h"
#ifdef __MYIMAGE_FILE_PNG__
#include "my1image_file_png.h"
#endif
#endif
#include "my1image_mask.h"
#include "my1image_area.h"
#include "my1image_buff.h"
#include "my1image_util.h"
#include "my1image_stat.h"
#ifdef __MYIMAGE_WORK__
#include "my1image_work.h"
#endif
#ifdef __MYIMAGE_VIEW__
#include "my1image_view.h"
#include "my1image_appw.h"
#include "my1image_hist.h"
#include "my1image_data.h"
#endif
#include "my1image_scan.h"
#include "my1image_mono.h"
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGEH__ */
/*----------------------------------------------------------------------------*/
