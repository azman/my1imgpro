/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MAIN_H__
#define __MY1IMAGE_MAIN_H__
/*----------------------------------------------------------------------------*/
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
#define MY1IMAIN_LOOP_DELAY 10
/*----------------------------------------------------------------------------*/
typedef struct _my1iwork_t
{
	my1itask_t init, free, args;
	my1itask_t prep, proc, show;
	pdata_t data;
}
my1iwork_t;
/*----------------------------------------------------------------------------*/
void iwork_make(my1iwork_t* work, pdata_t data); /* link all data! */
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
#include "my1image_util.h"
#include "my1image_grab.h"
/*----------------------------------------------------------------------------*/
#define BLANK_IMAGE_OPT "--blank"
#define BLANK_IMAGE_OPT_SIZE 8
/*----------------------------------------------------------------------------*/
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#define IFLAG_OK 0
#define IFLAG_ERROR (~(~0U>>1))
#define IFLAG_ERROR_2END (IFLAG_ERROR|0x010)
#define IFLAG_ERROR_ARGS (IFLAG_ERROR|0x020)
#define IFLAG_ERROR_LOAD (IFLAG_ERROR|0x040)
#define IFLAG_ERROR_LIST1 (IFLAG_ERROR|0x100)
#define IFLAG_ERROR_LIST2 (IFLAG_ERROR|0x200)
/*----------------------------------------------------------------------------*/
#define IFLAG_VIDEO_MODE 0x01
#define IFLAG_FILTER_RUN 0x02
#define IFLAG_FILTER_CHK 0x04
#define IFLAG_FILTER_EXE 0x08
/*----------------------------------------------------------------------------*/
typedef my1image_appw_t my1ishow_t;
typedef my1image_grab_t my1igrab_t;
typedef my1image_filter_t my1ipass_t;
typedef my1image_buffer_t my1ibuff_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1imain_t
{
	my1image_t load, *show;
	my1ishow_t iwin;
	my1igrab_t grab; /* grabber function */
	my1ibuff_t buff;
	int flag, mode, tdel;
	void* dovf; /* menu_item for filter flag */
	my1ipass_t *list, *curr; /* image processing filters */
	my1iwork_t *work;
}
my1imain_t;
/*----------------------------------------------------------------------------*/
void imain_init(my1imain_t* imain, my1iwork_t* iwork);
void imain_free(my1imain_t* imain);
void imain_args(my1imain_t* imain, int argc, char* argv[]);
void imain_prep(my1imain_t* imain);
void imain_proc(my1imain_t* imain);
void imain_show(my1imain_t* imain);
void imain_loop(my1imain_t* imain, int delta_ms);
void imain_menu_filter_enable(my1imain_t *imain, int enable);
void imain_domenu_filters(my1imain_t* imain);
my1ipass_t* imain_filter_dolist(my1imain_t*,filter_info_t*);
my1ipass_t* imain_filter_doload(my1imain_t*,char*);
my1ipass_t* imain_filter_unload(my1imain_t*,char*);
my1ipass_t* imain_filter_unload_all(my1imain_t*);
my1ipass_t* imain_filter_doload_list(my1imain_t*,char*);
my1ipass_t* imain_filter_doexec(my1imain_t* data);
/*----------------------------------------------------------------------------*/
void image_show(my1image_t* image, my1ishow_t* ishow, char* name);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_MAIN_H__ */
/*----------------------------------------------------------------------------*/
