/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEOH__
#define __MY1VIDEOH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
typedef unsigned char vbyte;
typedef struct
{
	vbyte b,g,r,a;
}
vrgb;
/*----------------------------------------------------------------------------*/
typedef struct _my1video_t
{
	my1image_t image; /* internal buffer */
	my1image_t *frame; /* pure pointer to buffered or filtered image */
	my1image_filter_t *filter; /* video/image filter */
	int count; /* frame count in video file (-1 for live?) */
	int index; /* frame index for video file */
	int width, height; /* set to desired size */
	vbyte looping; /* loop option */
	vbyte update; /* flag to activate frame grabber */
	vbyte stepit; /* frame step option */
	vbyte newframe; /* frame grabber should set this flag */
}
my1video_t;
/*----------------------------------------------------------------------------*/
void video_init(my1video_t *video);
void video_free(my1video_t *video);
/* these functions manipulate index/flags only! */
void video_play(my1video_t *video);
void video_hold(my1video_t *video);
void video_stop(my1video_t *video);
void video_next_frame(my1video_t *video);
void video_prev_frame(my1video_t *video);
/* filtering effect */
void video_filter(my1video_t *video);
/* for processing user requests */
void video_post_input(my1video_t *video);
/* color conversion helper functions */
int encode_vrgb(vrgb colorpix);
vrgb decode_vrgb(int rgbcode);
int vrgb2gray(vrgb colorpix);
vrgb gray2vrgb(int grayvalue);
int color2gray(int rgbcode);
int gray2color(int grayvalue);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
