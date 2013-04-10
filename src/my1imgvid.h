/*----------------------------------------------------------------------------*/
#ifndef __MY1IMGVIDH__
#define __MY1IMGVIDH__
/*----------------------------------------------------------------------------*/
#include "my1imgpro.h"
/*----------------------------------------------------------------------------*/
typedef unsigned char vbyte;
typedef struct
{
	vbyte b,g,r;
}
vrgb; /* TODO: may change this to YUV-based */
/*----------------------------------------------------------------------------*/
typedef my1Image* (*pVFilter)(my1Image* image, my1Image* result, void* userdata);
/*----------------------------------------------------------------------------*/
typedef struct _vfilter
{
	void *userdata;
	my1Image buffer;
	pVFilter filter;
	struct _vfilter *next; /* linked list */
}
my1VFilter;
/*----------------------------------------------------------------------------*/
struct _video
{
	my1Image *frame; /* pure pointer to buffered or filtered image */
	my1Image image; /* internal buffer */
	my1VFilter *filter; /* video/image filter */
	int count; /* frame count in video file (-1 for live?) */
	int index; /* frame index for video file */
	int width, height; /* set these to desired size! */
	vbyte looping; /* loop option */
	vbyte update; /* flag to activate frame grabber */
	vbyte stepit; /* frame step option */
	vbyte newframe; /* frame grabber should set this flag */
};
typedef struct _video my1Video;
/*----------------------------------------------------------------------------*/
void initvideo(my1Video *video);
void cleanvideo(my1Video *video);
/* these functions manipulate index/flags only! */
void playvideo(my1Video *video);
void pausevideo(my1Video *video);
void stopvideo(my1Video *video);
void nextvframe(my1Video *video);
void prevvframe(my1Video *video);
/* filtering effect */
void filtervideo(my1Video *video);
/* for processing user requests */
void postinput(my1Video *video);
// special converter
int encode_vrgb(vrgb colorpix);
vrgb decode_vrgb(int rgbcode);
int vrgb2gray(vrgb colorpix);
vrgb gray2vrgb(int grayvalue);
int color2gray(int rgbcode);
int gray2color(int grayvalue);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
