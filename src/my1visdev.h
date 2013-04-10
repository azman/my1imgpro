/*----------------------------------------------------------------------------*/
#ifndef __MY1VISDEVH__
#define __MY1VISDEVH__
/*----------------------------------------------------------------------------*/
#include "my1imgvid.h"
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
#include <SDL/SDL.h>
/*----------------------------------------------------------------------------*/
typedef struct SwsContext AVswsContext;
/*----------------------------------------------------------------------------*/
struct _capture
{
	AVFormatContext *fcontext;
	int vstream; /* stream index */
	AVCodecContext *ccontext;
	AVswsContext *rgb24fmt;
	uint8_t *pixbuf;
	AVFrame *frame; /* input frame */
	AVFrame *buffer; /* internal capture buffer (RGB) */
	AVFrame *ready; /* pure pointer to buffer */
	my1Video *video; /* pure pointer to video object */
	int lindex; /* sync marker for video index */
};
typedef struct _capture my1Capture;
/*----------------------------------------------------------------------------*/
void av2img(AVFrame* frame, my1Image* image);
void img2av(my1Image* image, AVFrame* frame);
void initcapture(my1Capture *object);
void cleancapture(my1Capture *object);
void filecapture(my1Capture *object, char *filename);
void livecapture(my1Capture *object, int camindex);
void grabcapture(my1Capture *object);
void stopcapture(my1Capture *object);
/*----------------------------------------------------------------------------*/
struct _display
{
	SDL_Surface *screen;
	SDL_Overlay *overlay;
	AVswsContext *yuv12fmt;
	uint8_t *pixbuf;
	AVFrame *buffer; /* internal display buffer (RGB) */
	SDL_Rect view;
	AVPicture pict; /* AV 'front-end' for SDL */
	my1Video *video; /* pure pointer to video object */
};
typedef struct _display my1Display;
/*----------------------------------------------------------------------------*/
void initdisplay(my1Display *object);
void cleandisplay(my1Display *object);
void setupdisplay(my1Display *object);
void buffdisplay(my1Display *object);
void showdisplay(my1Display *object);
void titledisplay(my1Display *object, const char *title, const char *icon);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
