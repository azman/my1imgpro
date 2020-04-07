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
	buffer_init(&video->vbuff);
	video->ppass = 0x0;
	video->count = -1; video->index = -1;
	video->width = 0; video->height = 0;
	video->flags = 0; /* reset flags */
	video->flags |= VIDEO_FLAG_LOOP; /* loop it! */
	video->flags |= VIDEO_FLAG_STEP; /* step it! */
	video->flags |= VIDEO_FLAG_NO_FILTER; /* do not filter initially */
	video->flags &= ~VIDEO_FLAG_DO_UPDATE; /* do not update initially */
	video->flags &= ~VIDEO_FLAG_NEW_FRAME; /* no new frame */
	video->flags &= ~VIDEO_FLAG_IS_PAUSED; /* NOT paused initially */
	video->flags &= ~VIDEO_FLAG_DO_GOBACK; /* disable reverse flag! */
	video->inkey = 0;
	video->capture = 0x0;
	video->display = 0x0;
	video->parent_ = 0x0;
}
/*----------------------------------------------------------------------------*/
void video_free(my1video_t *video)
{
	image_free(&video->image);
	buffer_free(&video->vbuff);
	/* assume filters are static OR cleared externally */
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
void video_skip_filter(my1video_t *video, int skip)
{
	if (skip) video->flags |= VIDEO_FLAG_NO_FILTER;
	else video->flags &= ~VIDEO_FLAG_NO_FILTER;
}
/*----------------------------------------------------------------------------*/
void video_filter_insert(my1video_t *video, my1vpass_t *vpass)
{
	my1vpass_t* ptask;
	while (vpass)
	{
		/* link buffer and parent */
		vpass->buffer = &video->vbuff;
		/* keep a copy to insert */
		ptask = vpass;
		vpass = vpass->next;
		/* insert video filter */
		video->ppass = filter_insert(video->ppass,ptask);
	}
}
/*----------------------------------------------------------------------------*/
void video_filter_frame(my1video_t *video)
{
	if (video->flags&VIDEO_FLAG_NO_FILTER) return;
	if (!video->ppass) return;
	video->frame = image_filter(video->frame,video->ppass);
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
/* -> taking these out! keeping for reference for now...
int encode_vrgb(vrgb_t colorpix)
{
	return encode_rgb(colorpix.r,colorpix.g,colorpix.b);
}
vrgb_t decode_vrgb(int rgbcode)
{
	vrgb_t cpix;
	cpix.a = 0;
	decode_rgb(rgbcode,&cpix.r,&cpix.g,&cpix.b);
	return cpix;
}
int vrgb2gray(vrgb_t colorpix)
{
	int *buff = (int*) &colorpix;
	return color2gray(*buff);
}
vrgb_t gray2vrgb(int grayvalue)
{
	int temp;
	vrgb_t *cpix = (vrgb_t*)&temp;
	temp = gray2color(grayvalue);
	return *cpix;
}
*/
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_BASEC__ */
/*----------------------------------------------------------------------------*/
