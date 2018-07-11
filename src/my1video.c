/*----------------------------------------------------------------------------*/
#include "my1video.h"
/*----------------------------------------------------------------------------*/
void video_init(my1video_t *video)
{
	image_init(&video->image);
	video->vdata.viewdata = &video->image;
	video->vdata.userdata = 0x0;
	video->frame = 0x0; /* frame is pure pointer! */
	video->filter = 0x0;
	video->count = -1; video->index = -1;
	video->width = 0; video->height = 0;
	video->looping = 0x1; video->update = 0x0;
	video->stepit = 0x1; video->newframe = 0x0;
}
/*----------------------------------------------------------------------------*/
void video_free(my1video_t *video)
{
	image_free(&video->image);
	/* clean up ALL filter buffer?? */
	my1image_filter_t *pfilter = video->filter;
	while (pfilter)
	{
		image_free(&pfilter->buffer);
		pfilter = pfilter->next;
	}
}
/*----------------------------------------------------------------------------*/
void video_play(my1video_t *video)
{
	video->update = 0x1;
	video->stepit = 0x0;
}
/*----------------------------------------------------------------------------*/
void video_hold(my1video_t *video)
{
	video->update = 0x0;
	video->stepit = 0x0;
}
/*----------------------------------------------------------------------------*/
void video_stop(my1video_t *video)
{
	video->update = 0x0;
	video->stepit = 0x0;
	if (video->index>0)
	{
		video->index = 0;
		/* get 1st frame */
		video->update = 0x1;
		video->stepit = 0x1;
	}
}
/*----------------------------------------------------------------------------*/
void video_next_frame(my1video_t *video)
{
	video->stepit = 0x1;
	if (video->index<video->count)
	{
		video->update = 0x1;
		video->index++;
		if (video->index==video->count)
		{
			if (video->looping)
			{
				video->index = 0;
			}
			else
			{
				video->update = 0x0;
				video->index--;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void video_prev_frame(my1video_t *video)
{
	video->stepit = 0x1;
	if (video->index>0)
	{
		video->update = 0x1;
		video->index--;
	}
}
/*----------------------------------------------------------------------------*/
void video_filter_init(my1video_t *video, my1vpass_t *vpass)
{
	my1vpass_t* ptask = vpass;
	while (ptask)
	{
		ptask->userdata = (void*) &video->vdata;
		ptask = ptask->next;
	}
	video->filter = vpass;
}
/*----------------------------------------------------------------------------*/
void video_filter(my1video_t *video)
{
	video->frame = image_filter(video->frame,video->filter);
}
/*----------------------------------------------------------------------------*/
void video_post_input(my1video_t *video)
{
	if (video->update&&!video->stepit)
	{
		video_next_frame(video);
		video->stepit = 0x0; /* override step in nextframe */
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
