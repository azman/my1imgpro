/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_MAINH__
#define __MY1VIDEO_MAINH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
#include "my1video_grab.h"
/*----------------------------------------------------------------------------*/
#define VIDEO_SOURCE_NONE 0x00
#define VIDEO_SOURCE_LIVE 0x01
#define VIDEO_SOURCE_FILE 0x02
/*----------------------------------------------------------------------------*/
#define MESG_SHOWTIME 5
#define VGRAB_DELAY 10
/*----------------------------------------------------------------------------*/
typedef void (*pgrabber_t)(void*);
typedef void (*chkinput_t)(void*);
/*----------------------------------------------------------------------------*/
typedef my1image_appw_t my1vappw_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1video_main_t
{
	my1video_t video;
	my1vgrab_t vgrab;
	my1vappw_t vappw;
	my1vpass_t *plist;
	guint ikey;
	int type, xdel;
	void* data; /* user data */
	pgrabber_t grabber; /* pointer to custom frame grabber function */
	void *grabber_data; /* data for grabber! */
	chkinput_t doinput; /* pointer to input capture function */
	void *doinput_data; /* data for chkinput! */
}
my1video_main_t;
/*----------------------------------------------------------------------------*/
typedef my1video_main_t my1vmain_t;
/*----------------------------------------------------------------------------*/
void video_main_init(my1vmain_t* vmain);
void video_main_free(my1vmain_t* vmain);
void video_main_capture(my1vmain_t* vmain, char* vsrc, int type);
void video_main_display(my1vmain_t* vmain, char* name);
void video_main_prepare(my1vmain_t* vmain);
void video_main_loop(my1vmain_t* vmain, int loopms);
void video_main_pass_more(my1vmain_t* vmain, filter_info_t* info);
void video_main_pass_load(my1vmain_t* vmain, char* name);
void video_main_pass_done(my1vmain_t* vmain);
/*----------------------------------------------------------------------------*/
/* example draw handler to show frame index */
void video_main_draw_index(void* data);
/* example handler to key input */
void video_main_check_input(void* data);
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_MAINH__ */
/*----------------------------------------------------------------------------*/
