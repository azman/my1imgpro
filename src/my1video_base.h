/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_BASEH__
#define __MY1VIDEO_BASEH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
typedef my1ipass_t my1vpass_t;
typedef my1ibuff_t my1vbuff_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_FLAG_LOOP 0x01
#define VIDEO_FLAG_STEP 0x02
#define VIDEO_FLAG_DO_UPDATE 0x04
#define VIDEO_FLAG_NEW_FRAME 0x08
#define VIDEO_FLAG_IS_PAUSED 0x10
#define VIDEO_FLAG_DO_GOBACK 0x20
/*----------------------------------------------------------------------------*/
typedef struct _my1video_t
{
	my1image_t image; /* internal buffer */
	my1image_t *frame; /* pure pointer to buffered or filtered image */
	int count; /* frame count in video file (-1 for live?) */
	int index; /* frame index for video file */
	int width, height; /* set to desired size */
	int flags; /* VIDEO_FLAGS_* */
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
/* execute this whenever a frame has been captured */
void video_post_frame(my1video_t *video);
/* handle user input */
void video_post_input(my1video_t *video);
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_BASEH__ */
/*----------------------------------------------------------------------------*/
