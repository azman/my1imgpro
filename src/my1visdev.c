/*----------------------------------------------------------------------------*/
#include "my1visdev.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
/*----------------------------------------------------------------------------*/
void av2img(AVFrame* frame, my1Image* image)
{
	int index, count;
	vrgb temp;
	image->mask = 0xffffff; /* rgb encoded int */
	for (index=0,count=0;index<image->length;index++)
	{
		temp.r = frame->data[0][count++];
		temp.g = frame->data[0][count++];
		temp.b = frame->data[0][count++];
		image->data[index] = encode_vrgb(temp);
	}
}
/*----------------------------------------------------------------------------*/
void img2av(my1Image* image, AVFrame* frame)
{
	int index,count;
	vrgb temp;
	for (index=0,count=0;index<image->length;index++)
	{
		if (image->mask)
			temp = decode_vrgb(image->data[index]);
		else
		{
			temp.r = temp.g = temp.b = image->data[index];
		}
		frame->data[0][count++] = temp.r;
		frame->data[0][count++] = temp.g;
		frame->data[0][count++] = temp.b;
	}
}
/*----------------------------------------------------------------------------*/
void initcapture(my1Capture *object)
{
	avcodec_register_all();
	av_register_all();
	avdevice_register_all();
	object->fcontext = 0x0;
	object->vstream = -1;
	object->ccontext = 0x0;
	object->rgb24fmt = 0x0;
	object->pixbuf = 0x0;
	object->strbuf = (uint8_t*)
		malloc(STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	memset((void*)object->strbuf,0,STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	object->packet = av_packet_alloc();
	object->frame = 0x0;
	object->buffer = 0x0;
	object->ready = 0x0;
	object->video = 0x0;
	object->lindex = -1;
}
/*----------------------------------------------------------------------------*/
void cleancapture(my1Capture *object)
{
	if (object->frame) av_free(object->frame);
	if (object->buffer) av_free(object->buffer);
	if (object->rgb24fmt) sws_freeContext(object->rgb24fmt);
	if (object->pixbuf) av_free(object->pixbuf);
	if (object->ccontext) avcodec_close(object->ccontext);
	if (object->fcontext) avformat_close_input(&object->fcontext);
	object->fcontext = 0x0;
}
/*----------------------------------------------------------------------------*/
void* grabframe(my1Capture *object)
{
	AVFormatContext *pFormatCtx = object->fcontext;
	AVCodecContext *pCodecCtx = object->ccontext;
	AVFrame *frame = 0x0;
	AVPacket packet;
	int test;
	if (!pFormatCtx) return frame;
	while (av_read_frame(pFormatCtx,&packet)>=0)
	{
		/* look for video stream packet */
		if (packet.stream_index==object->vstream)
		{
			/* decode video frame - NEW! */
			test = avcodec_send_packet(pCodecCtx,&packet);
			if (test>=0)
			{
				test = avcodec_receive_frame(pCodecCtx,object->frame);
				if (test>=0) /* 0: success */
				{
					/* did we get a complete frame? */
					frame = object->frame;
				}
/*
AVERROR(EAGAIN):
- output is not available in this state - user must try to send new input
AVERROR_EOF:
- the decoder has been fully flushed, and there will be no more output frames
AVERROR(EINVAL):
- codec not opened, or it is an encoder other negative values:
  legitimate decoding errors
*/
			}
		}
		/* free the packet alloced by av_read_frame */
		av_packet_unref(&packet);
		if (frame) break;
	}
	return frame;
}
/**
 * to drain codec internal buffers:
 * - avcodec_send_packet(NULL,NULL); => enter draining mode
 * - avcodec_receive_frame(); => loop until AVERROR_EOF returned
 * to reset codec to resum decoding:
 * - avcodec_flush_buffers();
 *
**/
/*----------------------------------------------------------------------------*/
void resetframe(my1Capture *object)
{
	AVFormatContext *pFormatCtx = object->fcontext;
	if (!pFormatCtx) return;
	if (avformat_seek_file(pFormatCtx, object->vstream,
		0, object->video->index, object->video->count-1,
		AVSEEK_FLAG_ANY|AVSEEK_FLAG_FRAME)<0)
	{
		printf("error while seeking frame %d in '%s'!\n",
			object->video->index,pFormatCtx->filename);
		exit(-1);
	}
	avcodec_flush_buffers(object->ccontext);
}
/*----------------------------------------------------------------------------*/
void formframe(my1Capture *object)
{
	/* convert to RGB! resize here as well? */
	sws_scale(object->rgb24fmt,
		(const uint8_t**) object->frame->data, object->frame->linesize,
		0, object->video->height,
		object->buffer->data, object->buffer->linesize);
}
/*----------------------------------------------------------------------------*/
void commcapture(my1Capture *object, char *doname)
{
	int loop, size;
	AVFormatContext *pFormatCtx = 0x0;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec = 0x0;
	AVStream *pStream;
	/* open resource - autodetect format & no dictionary */
	if (avformat_open_input(&pFormatCtx,doname,NULL,NULL))
	{
		printf("Cannot open resource %s\n",doname);
		exit(-1);
	}
	object->fcontext = pFormatCtx;
	/* retrieve stream information - no dictionary */
	if (avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Cannot find stream info in resource %s\n",doname);
		exit(-1);
	}
	/* find the best video stream - autodetect stream#, no pref, noflags */
	loop = av_find_best_stream(pFormatCtx,AVMEDIA_TYPE_VIDEO,-1,-1,&pCodec,0);
	if (loop<0)
	{
		av_dump_format(pFormatCtx,0,doname,0);
		if (loop==AVERROR_STREAM_NOT_FOUND)
		{
			printf("Cannot find video stream(s) in %s!\n",doname);
			exit(-1);
		}
		if (loop==AVERROR_DECODER_NOT_FOUND)
		{
			printf("Cannot find decoder for resource %s\n",doname);
			exit(-1);
		}
	}
	/* success! */
	object->vstream = loop;
	/* find the decoder for the video stream */
	pStream = pFormatCtx->streams[loop];
	pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
	if (pCodec==0x0)
	{
		av_dump_format(pFormatCtx,0,doname,0);
		printf("Cannot find suitable codec for file %s!\n",doname);
		exit(-1);
	}
	/* allocate memory for codec context */
	pCodecCtx = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(pCodecCtx,pStream->codecpar);
	object->ccontext = pCodecCtx;
	/* prepare codec */
	if (avcodec_open2(pCodecCtx,pCodec,NULL)<0)
	{
		printf("Cannot open codec for file %s!\n",doname);
		exit(-1);
	}
	/* check size requirements */
	if (!object->video->height) object->video->height = pCodecCtx->height;
	if (!object->video->width) object->video->width = pCodecCtx->width;
	/* create RGB24 converter context SWS_BILINEAR */
	object->rgb24fmt = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, object->video->width, object->video->height,
		AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	if (object->rgb24fmt==0x0)
	{
		printf("Cannot initialize the capture conversion context!\n");
		exit(-1);
	}
	/* allocate video frames */
	object->frame = av_frame_alloc();
	if (object->frame==0x0)
	{
		printf("Cannot allocate memory for input frame!\n");
		exit(-1);
	}
	object->buffer = av_frame_alloc();
	if (object->buffer==0x0)
	{
		printf("Cannot allocate memory for video frame!\n");
		exit(-1);
	}
	/* create RGB buffer - allocate the actual pixel buffer */
	size = av_image_get_buffer_size(AV_PIX_FMT_RGB24,object->video->width,
		object->video->height,32); /* 256bits/8 - 32-byte alignment! */
	object->pixbuf = (uint8_t *)av_malloc(size*sizeof(uint8_t));
	/** avpicture_fill((AVPicture*)object->buffer,object->pixbuf,
		AV_PIX_FMT_RGB24,object->video->width,object->video->height); */
	av_image_fill_arrays(object->buffer->data,object->buffer->linesize,
		object->pixbuf,AV_PIX_FMT_RGB24,
		object->video->width,object->video->height,1);
}
/*----------------------------------------------------------------------------*/
void filecapture(my1Capture *object, char *filename)
{
	/* check if captured... can we use this as captured flag? */
	if (object->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!object->video) return;
	/* try to initialize? */
	commcapture(object,filename);
	if (!object->fcontext) return;
	/* count frame? */
	object->video->count = 0;
	while(grabframe(object)) { object->video->count++; }
	if(!object->video->count)
	{
		printf("Cannot get frame count in '%s'?\n",filename);
		exit(-1);
	}
#ifdef MY1DEBUG
	printf("Frame Count for '%s': %d\n",filename,object->video->count);
#endif
	/** pCodecCtx->time_base has frame rate (struct with num/denom)! */
	/* setup config data */
	object->video->index = 0;
	object->video->update = 0x1;
	object->video->stepit = 0x1;
}
/*----------------------------------------------------------------------------*/
void livecapture(my1Capture *object, char *camname)
{
	/* check if captured... can we use this as captured flag? */
	if (object->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!object->video) return;
	/* try to initialize? */
	commcapture(object,camname);
	if (!object->fcontext) return;
	/* create/init device */
	object->video->count = -1; /* already default! */
	object->video->index = -1;
	object->video->update = 0x1;
	object->video->stepit = 0x0;
}
/*----------------------------------------------------------------------------*/
void grabcapture(my1Capture *object)
{
	if(!object->fcontext) return;
	if(!object->video) return;
	object->video->newframe = 0x0;
	object->ready = 0x0;
	if(!object->video->update) return;
	if(object->video->count<0) /* flag for livefeed */
	{
		//printf("Live feed not implemented... yet!\n");
		//exit(-1);
		object->ready = grabframe(object);
	}
	else
	{
		if(object->video->index==0) /* check reset request */
		{
			resetframe(object);
			object->lindex = -1;
		}
		/* using last index to see if grabbing is required */
		if(object->lindex<object->video->index)
		{
			object->ready = grabframe(object);
			object->lindex++;
		}
	}
	if(object->ready) /* create video internal copy if valid frame */
	{
		formframe(object); /* get frame in rgb format */
		if(!object->video->image.data)
			createimage(&object->video->image,
				object->video->height,object->video->width);
		av2img(object->buffer,&object->video->image);
		object->video->frame = &object->video->image;
		object->video->newframe = 0x1;
	}
	if(object->video->stepit) object->video->update = 0x0;
}
/*----------------------------------------------------------------------------*/
void stopcapture(my1Capture *object)
{
	if (!object->fcontext) return;
	av_free(object->frame);
	object->frame = 0x0;
	av_free(object->buffer);
	object->buffer = 0x0;
	av_free(object->pixbuf);
	object->pixbuf = 0x0;
	sws_freeContext(object->rgb24fmt);
	object->rgb24fmt = 0x0;
	avcodec_close(object->ccontext);
	object->ccontext = 0x0;
	avformat_close_input(&object->fcontext);
	object->fcontext = 0x0;
}
/*----------------------------------------------------------------------------*/
void initdisplay(my1Display *object)
{
	/* initialize SDL - audio not needed, actually! */
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		exit(-1);
	}
	object->screen = 0x0;
	object->overlay = 0x0;
	object->yuv12fmt = 0x0;
	object->pixbuf = 0x0;
	object->buffer = 0x0;
}
/*----------------------------------------------------------------------------*/
void cleandisplay(my1Display *object)
{
	if (object->yuv12fmt) sws_freeContext(object->yuv12fmt);
	if (object->pixbuf) av_free(object->pixbuf);
	if (object->buffer) av_free(object->buffer);
	SDL_Quit();
}
/*----------------------------------------------------------------------------*/
void setupdisplay(my1Display *object)
{
	int size;
	if(object->overlay) return;
	if (!object->video) return;
	/* create buffer frame */
	object->buffer = av_frame_alloc();
	if (object->buffer==0x0)
	{
		printf("Cannot allocate memory for display frame!\n");
		exit(-1);
	}
	/* create RGB buffer - allocate the actual pixel buffer */
	size = av_image_get_buffer_size(AV_PIX_FMT_RGB24,object->video->width,
		object->video->height,32); /* 256bits/8 - 32-byte alignment! */
	object->pixbuf = (uint8_t *)av_malloc(size*sizeof(uint8_t));
	av_image_fill_arrays(object->buffer->data,object->buffer->linesize,
		object->pixbuf,AV_PIX_FMT_RGB24,
		object->video->width,object->video->height,1);
	/* create format converter */
	if (!object->yuv12fmt)
	{
		object->yuv12fmt = sws_getContext(object->video->width, object->video->height,
			AV_PIX_FMT_RGB24, object->video->width, object->video->height,
			AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	}
	if (!object->yuv12fmt)
	{
		printf("Cannot initialize the display conversion context!\n");
		exit(-1);
	}
	/* save the size info here */
	object->view.x = 0;
	object->view.y = 0;
	object->view.h = object->video->height;
	object->view.w = object->video->width;
	/* create display screen & overlay */
	object->screen = SDL_SetVideoMode(object->video->width, object->video->height, 0, 0);
	if (!object->screen)
	{
		printf("SDL: could not set video mode - exiting\n");
		exit(-1);
	}
	object->overlay = SDL_CreateYUVOverlay(object->video->width, object->video->height,
		SDL_YV12_OVERLAY, object->screen); /* Y V U format! */
	/* assign ffmpeg object (AVPicture) to SDL object (overlay) */
	object->pict.data[0] = object->overlay->pixels[0];
	object->pict.data[1] = object->overlay->pixels[2]; /* switched because YVU? */
	object->pict.data[2] = object->overlay->pixels[1];
	/* linesize for each channel! */
	object->pict.linesize[0] = object->overlay->pitches[0];
	object->pict.linesize[1] = object->overlay->pitches[2];
	object->pict.linesize[2] = object->overlay->pitches[1];
}
/*----------------------------------------------------------------------------*/
void buffdisplay(my1Display *object)
{
	if (!object->overlay) return;
	if (!object->video) return;
	img2av(object->video->frame,object->buffer);
	SDL_LockYUVOverlay(object->overlay);
	sws_scale(object->yuv12fmt,
		(const uint8_t**) object->buffer->data, object->buffer->linesize,
		0, object->video->height,
		object->pict.data, object->pict.linesize);
	SDL_UnlockYUVOverlay(object->overlay);
}
/*----------------------------------------------------------------------------*/
void showdisplay(my1Display *object)
{
	if (!object->video) return;
	SDL_DisplayYUVOverlay(object->overlay, &object->view); /* blit op? */
}
/*----------------------------------------------------------------------------*/
void titledisplay(my1Display *object, const char *title, const char *icon)
{
	if (!object->video) return;
	SDL_WM_SetCaption(title,icon);
}
/*----------------------------------------------------------------------------*/
