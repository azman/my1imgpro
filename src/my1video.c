/*----------------------------------------------------------------------------*/
#include "my1video.h"
/*----------------------------------------------------------------------------*/
void video_init(my1video_t *video)
{
	image_init(&video->image);
	video->vdata.viewdata = &video->image;
	video->vdata.userdata = 0x0;
	video->frame = 0x0; /* frame is pure pointer! */
	buffer_init(&video->vbuff);
	video->filter = 0x0;
	video->count = -1; video->index = -1;
	video->width = 0; video->height = 0;
	video->flags = 0; /* reset flags */
	video->flags |= VIDEO_FLAG_LOOP; /* loop it! */
	video->flags |= VIDEO_FLAG_STEP; /* step it! */
	video->flags |= VIDEO_FLAG_NO_FILTER; /* do not filter initially */
	video->flags &= ~VIDEO_FLAG_DO_UPDATE; /* do not update initially */
	video->flags &= ~VIDEO_FLAG_NEW_FRAME; /* no new frame */
	video->flags &= ~VIDEO_FLAG_IS_PAUSED; /* NOT paused initially */
	video->inkey = 0;
	video->capture = 0x0;
	video->display = 0x0;
}
/*----------------------------------------------------------------------------*/
void video_free(my1video_t *video)
{
	image_free(&video->image);
	buffer_free(&video->vbuff);
}
/*----------------------------------------------------------------------------*/
void video_play(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_DO_UPDATE;
	video->flags &= ~VIDEO_FLAG_STEP;
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
	}
}
/*----------------------------------------------------------------------------*/
void video_next_frame(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_STEP;
	if (video->index<video->count)
	{
		video->flags |= VIDEO_FLAG_DO_UPDATE;
		video->index++;
		if (video->index==video->count)
		{
			if (video->flags&VIDEO_FLAG_LOOP)
			{
				video->index = 0;
			}
			else
			{
				video->flags &= ~VIDEO_FLAG_DO_UPDATE;
				video->index--;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void video_prev_frame(my1video_t *video)
{
	video->flags |= VIDEO_FLAG_STEP;
	if (video->index>0)
	{
		video->flags |= VIDEO_FLAG_DO_UPDATE;
		video->index--;
	}
}
/*----------------------------------------------------------------------------*/
void video_filter_init(my1video_t *video, my1vpass_t *vpass)
{
	my1vpass_t* ptask = vpass;
	while (ptask)
	{
		/* allow filters to use external buffer */
		if (!ptask->buffer) ptask->buffer = &video->vbuff;
		ptask->userdata = (void*) &video->vdata;
		ptask = ptask->next;
	}
	video->filter = vpass;
}
/*----------------------------------------------------------------------------*/
void video_filter(my1video_t *video)
{
	if (video->flags&VIDEO_FLAG_NO_FILTER) return;
	video->frame = image_filter(video->frame,video->filter);
}
/*----------------------------------------------------------------------------*/
void video_post_frame(my1video_t *video)
{
	if (video->flags&VIDEO_FLAG_IS_PAUSED) return;
	if (video->flags&VIDEO_FLAG_STEP) return;
	if (video->flags&VIDEO_FLAG_DO_UPDATE)
	{
		video_next_frame(video);
		video->flags &= ~VIDEO_FLAG_STEP; /* make sure NOT stepping */
	}
}
/*----------------------------------------------------------------------------*/
int encode_vrgb(vrgb colorpix)
{
	return encode_rgb(colorpix.r,colorpix.g,colorpix.b);
}
/*----------------------------------------------------------------------------*/
vrgb decode_vrgb(int rgbcode)
{
	vrgb cpix;
	decode_rgb(rgbcode,&cpix.r,&cpix.g,&cpix.b);
	return cpix;
}
/*----------------------------------------------------------------------------*/
int vrgb2gray(vrgb colorpix)
{
	return ((int)colorpix.b+colorpix.g+colorpix.r)/3;
}
/*----------------------------------------------------------------------------*/
vrgb gray2vrgb(int grayvalue)
{
	vrgb cpix;
	cpix.b = (vbyte) grayvalue;
	cpix.g = (vbyte) grayvalue;
	cpix.r = (vbyte) grayvalue;
	return cpix;
}
/*----------------------------------------------------------------------------*/
