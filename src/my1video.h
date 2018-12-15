/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEOH__
#define __MY1VIDEOH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
typedef unsigned char vbyte;
typedef my1image_filter_t my1vpass_t;
typedef my1image_buffer_t my1vbuff_t;
/*----------------------------------------------------------------------------*/
typedef struct
{
	vbyte b,g,r,a;
}
vrgb;
/*----------------------------------------------------------------------------*/
typedef struct _my1vdata_t
{
	my1image_t *viewdata;
	void *userdata;
}
my1vdata_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_FLAG_LOOP 0x01
#define VIDEO_FLAG_STEP 0x02
#define VIDEO_FLAG_DO_UPDATE 0x04
#define VIDEO_FLAG_NEW_FRAME 0x08
#define VIDEO_FLAG_NO_FILTER 0x10
/*----------------------------------------------------------------------------*/
typedef struct _my1video_t
{
	my1image_t image; /* internal buffer */
	my1image_t *frame; /* pure pointer to buffered or filtered image */
	my1vdata_t vdata; /* summary link :p */
	my1vbuff_t vbuff; /* video double buffering for video filter */
	my1vpass_t *filter; /* video/image filter */
	int count; /* frame count in video file (-1 for live?) */
	int index; /* frame index for video file */
	int width, height; /* set to desired size */
	int flags; /* VIDEO_FLAGS_* */
	int inkey; /* input key? */
	void *display; /* pointer to display object */
	void *capture; /* pointer to capture object */
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
void video_filter_init(my1video_t *video,my1vpass_t *vpass);
void video_filter(my1video_t *video);
/* for streaming frames */
void video_post_frame(my1video_t *video);
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
