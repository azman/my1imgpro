/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_DEVH__
#define __MY1VIDEO_DEVH__
/*----------------------------------------------------------------------------*/
#include "my1video.h"
/*----------------------------------------------------------------------------*/
#include <libavformat/avformat.h>
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_TITLE "MY1 Video Tool"
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
	guint timer;
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
void display_name(my1video_display_t* vview,
	const char *name,const char *icon);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
