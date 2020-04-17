/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_BASEC__
#define __MY1VIDEO_BASEC__
/*----------------------------------------------------------------------------*/
#include "my1video_base.h"
/*----------------------------------------------------------------------------*/
void video_init(my1video_t *video)
{
	image_init(&video->image);
	video->frame = 0x0; /* frame is pure pointer! */
	video->count = -1; video->index = -1;
	video->width = 0; video->height = 0;
	video->flags = 0; /* reset flags */
	video->flags |= VIDEO_FLAG_LOOP; /* loop it! */
	video->flags |= VIDEO_FLAG_STEP; /* step it! */
	video->flags &= ~VIDEO_FLAG_DO_UPDATE; /* do not update initially */
	video->flags &= ~VIDEO_FLAG_NEW_FRAME; /* no new frame */
	video->flags &= ~VIDEO_FLAG_IS_PAUSED; /* NOT paused initially */
	video->flags &= ~VIDEO_FLAG_DO_GOBACK; /* disable reverse flag! */
}
/*----------------------------------------------------------------------------*/
void video_free(my1video_t *video)
{
	image_free(&video->image);
}
/*----------------------------------------------------------------------------*/
void video_play(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_DO_UPDATE;
	video->flags &= ~VIDEO_FLAG_STEP;
	video->flags &= ~VIDEO_FLAG_IS_PAUSED;
}
/*----------------------------------------------------------------------------*/
void video_stop(my1video_t *video)
{
	video->flags &= ~VIDEO_FLAG_DO_UPDATE;
	video->flags &= ~VIDEO_FLAG_STEP;
	if (video->index>0)
	{
		video->index = 0;
		/* get 1st frame and step on */
		video->flags |= VIDEO_FLAG_DO_UPDATE;
		video->flags |= VIDEO_FLAG_STEP;
		video->flags |= VIDEO_FLAG_IS_PAUSED;
	}
}
/*----------------------------------------------------------------------------*/
void video_hold(my1video_t *video, int hold)
{
	if (hold)
	{
		video->flags |= VIDEO_FLAG_IS_PAUSED;
		video->flags &= ~VIDEO_FLAG_DO_UPDATE;
	}
	else
	{
		video->flags &= ~VIDEO_FLAG_IS_PAUSED;
		video->flags &= ~VIDEO_FLAG_STEP;
		video->flags |= VIDEO_FLAG_DO_UPDATE;
	}
}
/*----------------------------------------------------------------------------*/
void video_loop(my1video_t *video, int loop)
{
	if (loop) video->flags |= VIDEO_FLAG_LOOP;
	else video->flags &= ~VIDEO_FLAG_LOOP;
}
/*----------------------------------------------------------------------------*/
void video_next_frame(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_STEP;
	video->flags |= VIDEO_FLAG_IS_PAUSED;
	video->flags &= ~VIDEO_FLAG_DO_GOBACK;
	video->flags |= VIDEO_FLAG_DO_UPDATE;
}
/*----------------------------------------------------------------------------*/
void video_prev_frame(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_STEP;
	video->flags |= VIDEO_FLAG_IS_PAUSED;
	video->flags |= VIDEO_FLAG_DO_GOBACK;
	video->flags |= VIDEO_FLAG_DO_UPDATE;
}
/*----------------------------------------------------------------------------*/
void video_post_frame(my1video_t *video)
{
	if (video->count<0) return;
	video->index++;
	if (video->index>video->count)
	{
		if (video->flags&VIDEO_FLAG_LOOP)
		{
			video->index = 0;
		}
		else
		{
			video->flags &= ~VIDEO_FLAG_DO_UPDATE;
			video->flags |= VIDEO_FLAG_IS_PAUSED;
		}
	}
}
/*----------------------------------------------------------------------------*/
void video_post_input(my1video_t *video)
{
	if (video->count<0) return;
	if (video->flags&VIDEO_FLAG_DO_UPDATE)
	{
		if (video->flags&VIDEO_FLAG_DO_GOBACK)
		{
			video->flags &= ~VIDEO_FLAG_DO_GOBACK;
			if (video->count>=0&&video->index>1)
				video->index -= 2;
			else
			{
				video->flags &= ~VIDEO_FLAG_DO_UPDATE;
				video->flags |= VIDEO_FLAG_IS_PAUSED;
			}
		}
		if (video->index>=video->count)
		{
			if (video->flags&VIDEO_FLAG_LOOP)
			{
				video->index = 0;
			}
			else
			{
				video->flags &= ~VIDEO_FLAG_DO_UPDATE;
				video->flags |= VIDEO_FLAG_IS_PAUSED;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_BASEC__ */
/*----------------------------------------------------------------------------*/
