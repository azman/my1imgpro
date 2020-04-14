/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MAIN_H__
#define __MY1IMAGE_MAIN_H__
/*----------------------------------------------------------------------------*/
/* that is usually the main data structure */
typedef int (*my1itask_t)(void* data, void* that, void* xtra);
typedef void* my1idata_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1iwork_t
{
	my1itask_t init, free, args;
	my1itask_t prep, proc, show;
	my1idata_t data;
}
my1iwork_t;
/*----------------------------------------------------------------------------*/
int iwork_make(my1iwork_t* work, my1idata_t data);
int iwork_init(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
int iwork_free(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
int iwork_args(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
int iwork_prep(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
int iwork_proc(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
int iwork_show(my1iwork_t* work, my1idata_t that, my1idata_t xtra);
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#define IFLAG_OK 0
#define IFLAG_ERROR (~(~0U>>1))
#define IFLAG_ERROR_ARGS (IFLAG_ERROR|0x01)
#define IFLAG_ERROR_LOAD (IFLAG_ERROR|0x02)
#define IFLAG_ERROR_LIST1 (IFLAG_ERROR|0x04)
#define IFLAG_ERROR_LIST2 (IFLAG_ERROR|0x08)
/*----------------------------------------------------------------------------*/
typedef my1image_appw_t my1ishow_t;
typedef my1image_filter_t my1ipass_t;
typedef my1image_buffer_t my1ibuff_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1imain_t
{
	my1image_t main, temp, *show, *orig;
	my1ishow_t iwin;
	my1ipass_t pass; /* image pre-filter */
	my1ibuff_t buff;
	int flag;
	char* pick;
	my1ipass_t *list, *curr, *pchk; /* image processing filters */
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
void imain_filter_dolist(my1imain_t* data, filter_info_t* info);
void imain_filter_doload(my1imain_t* data, char* name);
void imain_filter_unload(my1imain_t* data, char* name);
void imain_filter_enable(my1imain_t* data, int enable);
void imain_filter_doexec(my1imain_t* data);
/*----------------------------------------------------------------------------*/
void image_show(my1image_t* image, my1ishow_t* ishow, char* name);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_MAIN_H__ */
/*----------------------------------------------------------------------------*/
