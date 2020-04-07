/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_GRABH__
#define __MY1VIDEO_GRABH__
/*----------------------------------------------------------------------------*/
#include "my1video_base.h"
/*----------------------------------------------------------------------------*/
#include <libavformat/avformat.h>
/*----------------------------------------------------------------------------*/
typedef struct SwsContext AVswsContext;
/*----------------------------------------------------------------------------*/
typedef struct _my1video_capture_t
{
	AVFormatContext *fcontext;
	AVCodecContext *ccontext;
	AVPacket *packet; /* packet decoding */
	AVswsContext *rgb32fmt;
	uint8_t *pixbuf; /* actual RGB buffer */
	AVFrame *buffer; /* frame for internal capture buffer (RGB) */
	AVFrame *frame; /* input frame */
	AVFrame *ready; /* pure pointer to buffer */
	my1video_t *video; /* pure pointer to video object */
	int index; /* sync marker for video index */
	int vstream; /* stream index */
}
my1video_capture_t;
/*----------------------------------------------------------------------------*/
typedef my1video_capture_t my1video_grab_t;
typedef my1video_grab_t my1vgrab_t;
/*----------------------------------------------------------------------------*/
void capture_init(my1video_capture_t* vgrab, my1video_t* video);
void capture_free(my1video_capture_t* vgrab);
void capture_file(my1video_capture_t* vgrab, char *filename);
void capture_live(my1video_capture_t* vgrab, char *camname);
void capture_grab(my1video_capture_t* vgrab);
void capture_stop(my1video_capture_t* vgrab);
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_GRABH__ */
/*----------------------------------------------------------------------------*/
