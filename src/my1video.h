/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEOH__
#define __MY1VIDEOH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
typedef unsigned char vbyte;
typedef my1image_filter_t my1vpass_t;
typedef my1image_buffer_t my1vbuff_t;
typedef my1image_rgb_t vrgb_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_FLAG_LOOP 0x01
#define VIDEO_FLAG_STEP 0x02
#define VIDEO_FLAG_DO_UPDATE 0x04
#define VIDEO_FLAG_NEW_FRAME 0x08
#define VIDEO_FLAG_NO_FILTER 0x10
#define VIDEO_FLAG_IS_PAUSED 0x20
#define VIDEO_FLAG_DO_GOBACK 0x40
/*----------------------------------------------------------------------------*/
typedef struct _my1video_t
{
	my1image_t image; /* internal buffer */
	my1image_t *frame; /* pure pointer to buffered or filtered image */
	my1vbuff_t vbuff; /* video double buffering for video filter */
	my1vpass_t *ppass; /* video/image filter */
	int count; /* frame count in video file (-1 for live?) */
	int index; /* frame index for video file */
	int width, height; /* set to desired size */
	int flags; /* VIDEO_FLAGS_* */
	int inkey; /* input key? */
	void *display; /* pointer to display object */
	void *capture; /* pointer to capture object */
	void *parent_; /* pointer to parent object */
}
my1video_t;
/*----------------------------------------------------------------------------*/
void video_init(my1video_t *video);
void video_free(my1video_t *video);
/* these functions manipulate index/flags only! */
void video_play(my1video_t *video);
void video_stop(my1video_t *video);
void video_hold(my1video_t *video, int hold);
void video_loop(my1video_t *video, int loop);
void video_next_frame(my1video_t *video);
void video_prev_frame(my1video_t *video);
void video_skip_filter(my1video_t *video, int skip);
/* filtering effect */
void video_filter_insert(my1video_t *video,my1vpass_t *vpass);
void video_filter_frame(my1video_t *video);
/* execute this whenever a frame has been captured */
void video_post_frame(my1video_t *video);
/* handle user input */
void video_post_input(my1video_t *video);
/* color conversion helper functions */
int encode_vrgb(vrgb_t colorpix);
vrgb_t decode_vrgb(int rgbcode);
int vrgb2gray(vrgb_t colorpix);
vrgb_t gray2vrgb(int grayvalue);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
