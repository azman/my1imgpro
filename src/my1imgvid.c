/*
	my1imgvid => video image processing based on my1imgpro
	Author: azman@my1matrix.net
*/

#include "my1imgvid.h"

void initvideo(my1Video *video)
{
	video->frame = 0x0; /* frame is pure pointer! */
	video->image.data = 0x0;
	video->filter = 0x0;
	video->count = -1; video->index = -1;
	video->width = 0; video->height = 0;
	video->looping = 0x1; video->update = 0x0;
	video->stepit = 0x1; video->newframe = 0x0;
}

void cleanvideo(my1Video *video)
{
	video->frame = 0x0; /* frame is pure pointer! */
	if(video->image.data) freeimage(&video->image);
	/* clean up ALL filter buffer?? */
	my1VFilter *pfilter = video->filter;
	while(pfilter)
	{
		if(pfilter->buffer.data) freeimage(&pfilter->buffer);
		pfilter = pfilter->next;
	}
}

void playvideo(my1Video *video)
{
	video->update = 0x1;
	video->stepit = 0x0;
}

void pausevideo(my1Video *video)
{
	video->update = 0x0;
	video->stepit = 0x0;
}

void stopvideo(my1Video *video)
{
	video->update = 0x0;
	video->stepit = 0x0;
	if(video->index>0)
	{
		video->index = 0;
		/* get 1st frame */
		video->update = 0x1;
		video->stepit = 0x1;
	}
}

void nextvframe(my1Video *video)
{
	video->stepit = 0x1;
	if(video->index<video->count)
	{
		video->update = 0x1;
		video->index++;
		if(video->index==video->count)
		{
			if(video->looping)
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

void prevvframe(my1Video *video)
{
	video->stepit = 0x1;
	if(video->index>0)
	{
		video->update = 0x1;
		video->index--;
	}
}

void filtervideo(my1Video *video)
{
	my1VFilter *pfilter = video->filter;
	while(pfilter)
	{
		if(pfilter->filter)
			video->frame = pfilter->filter(video->frame,&pfilter->buffer,pfilter->userdata);
		pfilter = pfilter->next;
	}
}

void postinput(my1Video *video)
{
	if(video->update&&!video->stepit)
	{
		nextvframe(video);
		video->stepit = 0x0; // override step in nextframe
	}
}

int encode_vrgb(vrgb colorpix)
{
	return (((int)colorpix.r)<<16|((int)colorpix.g)<<8|(int)colorpix.b);
}

vrgb decode_vrgb(int rgbcode)
{
	vrgb cpix;
	cpix.b = (rgbcode&0xff);
	cpix.g = (rgbcode&0xff00)>>8;
	cpix.r = (rgbcode&0xff0000)>>16;
	return cpix;
}

int vrgb2gray(vrgb colorpix)
{
	return ((int)colorpix.b+colorpix.g+colorpix.r)/3;
}

vrgb gray2vrgb(int grayvalue)
{
	vrgb cpix;
	cpix.b = (vbyte) grayvalue;
	cpix.g = (vbyte) grayvalue;
	cpix.r = (vbyte) grayvalue;
	return cpix;
}

int color2gray(int rgbcode)
{
	int b = (rgbcode&0xff);
	int g = (rgbcode&0xff00)>>8;
	int r = (rgbcode&0xff0000)>>16;
	return (b+g+r)/3;
}

int gray2color(int grayvalue)
{
	return ((grayvalue&0xff)<<16|(grayvalue&0xff)<<8|(grayvalue&0xff));
}
