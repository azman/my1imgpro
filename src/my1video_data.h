/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_MAINH__
#define __MY1VIDEO_MAINH__
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
#include "my1image_grab.h"
#include "my1video_base.h"
#include "my1libav_grab.h"
/*----------------------------------------------------------------------------*/
#define MESG_SHOWTIME 5
/*----------------------------------------------------------------------------*/
#define VIDEO_SOURCE_NONE 0x00
#define VIDEO_SOURCE_LIVE 0x01
#define VIDEO_SOURCE_FILE 0x02
/*----------------------------------------------------------------------------*/
typedef my1libav_grab_t my1vgrab_t;
typedef my1image_appw_t my1vappw_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1video_data_t
{
	my1vgrab_t vgrab;
	my1vappw_t vappw; /* thumbnail source image? */
	my1video_t video; /* manage image stream can reverse frame? */
	int flag, loop, argc;
	char** argv; /* to load filters later... */
}
my1video_data_t;
/*----------------------------------------------------------------------------*/
typedef my1video_data_t my1vdata_t;
/*----------------------------------------------------------------------------*/
int video_data_init(void* data, void* that, void* xtra);
int video_data_free(void* data, void* that, void* xtra);
int video_data_args(void* data, void* that, void* xtra);
int video_data_prep(void* data, void* that, void* xtra);
int video_data_exec(void* data, void* that, void* xtra);
int video_data_show(void* data, void* that, void* xtra);
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_MAINH__ */
/*----------------------------------------------------------------------------*/
