/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_DEVH__
#define __MY1VIDEO_DEVH__
/*----------------------------------------------------------------------------*/
#include "my1video.h"
/*----------------------------------------------------------------------------*/
#include <libavformat/avformat.h>
#include <SDL/SDL.h>
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
	AVswsContext *rgb24fmt;
	uint8_t *pixbuf, *strbuf; /* pixel buffer & stream buffer */
	AVPacket *packet;
	AVFrame *frame; /* input frame */
	AVFrame *buffer; /* internal capture buffer (RGB) */
	AVFrame *ready; /* pure pointer to buffer */
	my1video_t *video; /* pure pointer to video object */
	int lindex; /* sync marker for video index */
}
my1video_capture_t;
/*----------------------------------------------------------------------------*/
void image_get_frame(my1image_t* image, AVFrame* frame);
void image_set_frame(my1image_t* image, AVFrame* frame);
void capture_init(my1video_capture_t* object);
void capture_free(my1video_capture_t* object);
void capture_file(my1video_capture_t* object, char *filename);
void capture_live(my1video_capture_t* object, char *camname);
void capture_grab(my1video_capture_t* object);
void capture_stop(my1video_capture_t* object);
/*----------------------------------------------------------------------------*/
typedef struct _my1video_display_t
{
	SDL_Surface *screen;
	SDL_Overlay *overlay;
	AVswsContext *yuv12fmt;
	uint8_t *pixbuf;
	AVFrame *buffer; /* internal display buffer (RGB) */
	SDL_Rect view;
	AVFrame pict; /* AV 'front-end' for SDL */
	my1video_t *video; /* pure pointer to video object */
}
my1video_display_t;
/*----------------------------------------------------------------------------*/
void display_init(my1video_display_t* object);
void display_free(my1video_display_t* object);
void display_make(my1video_display_t* object);
void display_buff(my1video_display_t* object);
void display_view(my1video_display_t* object);
void display_name(my1video_display_t* object,
	const char *name,const char *icon);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
