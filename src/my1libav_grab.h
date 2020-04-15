/*----------------------------------------------------------------------------*/
#ifndef __MY1LIBAV_GRAB_H__
#define __MY1LIBAV_GRAB_H__
/*----------------------------------------------------------------------------*/
#include "my1image_base.h"
/*----------------------------------------------------------------------------*/
#include <libavformat/avformat.h>
/*----------------------------------------------------------------------------*/
typedef struct SwsContext AVswsContext;
/*----------------------------------------------------------------------------*/
typedef struct _my1libav_grab_t
{
	AVFormatContext *fcontext, *flag;
	AVCodecContext *ccontext;
	AVPacket *packet; /* packet decoding */
	AVswsContext *rgb32fmt;
	uint8_t *pixbuf; /* actual RGB buffer */
	AVFrame *buffer; /* frame for internal capture buffer (RGB) */
	AVFrame *frame; /* input frame */
	AVFrame *ready; /* pure pointer to buffer */
	my1image_t *image; /* target to place grabbed image */
	int width, height;
	int count; /* video file frame count  */
	int index; /* sync marker for video index */
	int vstream; /* stream index */
}
my1libav_grab_t;
/*----------------------------------------------------------------------------*/
void libav1_init(my1libav_grab_t* vgrab, my1image_t* image);
void libav1_free(my1libav_grab_t* vgrab);
void libav1_file(my1libav_grab_t* vgrab, char *filename);
void libav1_live(my1libav_grab_t* vgrab, char *camname);
void libav1_grab(my1libav_grab_t* vgrab, my1image_t* image);
/*----------------------------------------------------------------------------*/
#endif /** __MY1LIBAV_GRAB_H__ */
/*----------------------------------------------------------------------------*/
