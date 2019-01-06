/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_MAINH__
#define __MY1VIDEO_MAINH__
/*----------------------------------------------------------------------------*/
#include "my1video.h"
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
#include <libavformat/avformat.h>
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
typedef struct SwsContext AVswsContext;
/*----------------------------------------------------------------------------*/
#define STRBUF_SIZE 4096
/*----------------------------------------------------------------------------*/
typedef struct _my1video_capture_t
{
	AVFormatContext *fcontext;
	int vstream; /* stream index */
	AVCodecContext *ccontext;
	AVswsContext *rgb32fmt;
	uint8_t *pixbuf, *strbuf; /* pixel buffer & stream buffer */
	AVPacket *packet;
	AVFrame *frame; /* input frame */
	AVFrame *buffer; /* internal capture buffer (RGB) */
	AVFrame *ready; /* pure pointer to buffer */
	my1video_t *video; /* pure pointer to video object */
	int index; /* sync marker for video index */
}
my1video_capture_t;
/*----------------------------------------------------------------------------*/
void image_get_frame(my1image_t* image, AVFrame* frame);
void image_set_frame(my1image_t* image, AVFrame* frame);
void capture_init(my1video_capture_t* vgrab, my1video_t* video);
void capture_free(my1video_capture_t* vgrab);
void capture_file(my1video_capture_t* vgrab, char *filename);
void capture_live(my1video_capture_t* vgrab, char *camname);
void capture_grab(my1video_capture_t* vgrab);
void capture_stop(my1video_capture_t* vgrab);
/*----------------------------------------------------------------------------*/
#define FRAME_NEXT_MS 10
#define MESG_SHOWTIME 1
#define DEFAULT_LOOP_TIME 0
/*----------------------------------------------------------------------------*/
typedef void (*pchkinput)(void*);
/*----------------------------------------------------------------------------*/
typedef struct _my1video_display_t
{
	GdkPixbuf *pixbuf;
	guint timer,inkey;
	int delms;
	pchkinput chkinput;
	void* chkinput_data;
	my1video_t *video; /* pure pointer to video object */
	my1image_view_t view;
}
my1video_display_t;
/*----------------------------------------------------------------------------*/
void display_init(my1video_display_t* vview, my1video_t* video);
void display_free(my1video_display_t* vview);
void display_make(my1video_display_t* vview);
void display_loop(my1video_display_t* vview, int delms);
void display_draw(my1video_display_t* vview);
void display_name(my1video_display_t* vview, const char *name,const char *icon);
/*----------------------------------------------------------------------------*/
typedef my1video_capture_t my1vgrab_t;
typedef my1video_display_t my1vview_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_SOURCE_NONE 0x00
#define VIDEO_SOURCE_LIVE 0x01
#define VIDEO_SOURCE_FILE 0x02
/*----------------------------------------------------------------------------*/
typedef void (*pgrabber)(void*);
/*----------------------------------------------------------------------------*/
typedef struct _my1video_main_t
{
	my1video_t video;
	my1vgrab_t vgrab;
	my1vview_t vview;
	my1vpass_t *plist;
	int type;
	void* data;
	pgrabber grabber; /* pointer to custom frame grabber function */
	void *grabber_data; /* data for grabbber! */
}
my1video_main_t;
/*----------------------------------------------------------------------------*/
typedef my1video_main_t my1vmain_t;
/*----------------------------------------------------------------------------*/
void video_main_init(my1vmain_t* vmain);
void video_main_free(my1vmain_t* vmain);
void video_main_capture(my1vmain_t* vmain, char* vsrc, int type);
void video_main_display(my1vmain_t* vmain, char* name);
void video_main_loop(my1vmain_t* vmain, int loopms);
void video_main_pass_load(my1vmain_t* vmain, char* name);
void video_main_pass_done(my1vmain_t* vmain);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
