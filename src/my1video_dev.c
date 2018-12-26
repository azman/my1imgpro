/*----------------------------------------------------------------------------*/
#include "my1video_dev.h"
/*----------------------------------------------------------------------------*/
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
/*----------------------------------------------------------------------------*/
void image_get_frame(my1image_t* image, AVFrame* frame)
{
	int index,count;
	vrgb_t temp;
	for (index=0,count=0;index<image->length;index++)
	{
		if (image->mask)
			temp = decode_vrgb(image->data[index]);
		else
		{
			temp.r = temp.g = temp.b = image->data[index];
		}
		frame->data[0][count++] = temp.b;
		frame->data[0][count++] = temp.g;
		frame->data[0][count++] = temp.r;
		count++; /* ignore alpha */
	}
}
/*----------------------------------------------------------------------------*/
void image_set_frame(my1image_t* image, AVFrame* frame)
{
	int index, count;
	vrgb_t temp;
	for (index=0,count=0;index<image->length;index++)
	{
		temp.b = frame->data[0][count++];
		temp.g = frame->data[0][count++];
		temp.r = frame->data[0][count++];
		count++; /* ignore alpha */
		image->data[index] = encode_vrgb(temp);
	}
}
/*----------------------------------------------------------------------------*/
void capture_init(my1video_capture_t* object, my1video_t* video)
{
	avcodec_register_all();
	av_register_all();
	avdevice_register_all();
	object->fcontext = 0x0;
	object->vstream = -1;
	object->ccontext = 0x0;
	object->rgb32fmt = 0x0;
	object->pixbuf = 0x0;
	object->strbuf = (uint8_t*)
		malloc(STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	memset((void*)object->strbuf,0,STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	object->packet = av_packet_alloc();
	object->frame = 0x0;
	object->buffer = 0x0;
	object->ready = 0x0;
	object->video = video;
	object->index = -1;
	video->capture = (void*) object;
}
/*----------------------------------------------------------------------------*/
void capture_free(my1video_capture_t* object)
{
	if (object->frame) av_free(object->frame);
	if (object->buffer) av_free(object->buffer);
	if (object->rgb32fmt) sws_freeContext(object->rgb32fmt);
	if (object->pixbuf) av_free(object->pixbuf);
	if (object->ccontext) avcodec_close(object->ccontext);
	if (object->fcontext) avformat_close_input(&object->fcontext);
}
/*----------------------------------------------------------------------------*/
void* capture_grab_frame(my1video_capture_t* object)
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
void capture_reset(my1video_capture_t* object)
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
void capture_form_frame(my1video_capture_t* object)
{
	/* convert to RGB! resize here as well? */
	sws_scale(object->rgb32fmt,
		(const uint8_t**) object->frame->data, object->frame->linesize,
		0, object->video->height,
		object->buffer->data, object->buffer->linesize);
}
/*----------------------------------------------------------------------------*/
void capture_core(my1video_capture_t* object, char *doname)
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
	/* create RGB32 converter context */
	object->rgb32fmt = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, object->video->width, object->video->height,
		AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	if (object->rgb32fmt==0x0)
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
	size = av_image_get_buffer_size(AV_PIX_FMT_RGB32,object->video->width,
		object->video->height,32); /* byte alignment! */
	object->pixbuf = (uint8_t *)av_malloc(size*sizeof(uint8_t));
	av_image_fill_arrays(object->buffer->data,object->buffer->linesize,
		object->pixbuf,AV_PIX_FMT_RGB32,
		object->video->width,object->video->height,1);
}
/*----------------------------------------------------------------------------*/
void capture_file(my1video_capture_t* object, char *filename)
{
	/* check if captured... can we use this as captured flag? */
	if (object->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!object->video) return;
	/* try to initialize? */
	capture_core(object,filename);
	if (!object->fcontext) return;
	/* count frame? */
	object->video->count = 0;
	/* grab a 'sample' frame */
	if (capture_grab_frame(object))
	{
		capture_form_frame(object); /* get frame in rgb format */
		image_make(&object->video->image,
			object->video->height,object->video->width);
		/* capture_form_frame always convert rgb? */
		object->video->image.mask = IMASK_COLOR24;
		image_set_frame(&object->video->image,object->buffer);
		object->video->frame = &object->video->image;
		do
		{
			object->video->count++;
		}
		while (capture_grab_frame(object));
	}
	else
	{
		printf("Cannot get frames from '%s'?\n",filename);
		exit(-1);
	}
#ifdef MY1DEBUG
	printf("Frame Count for '%s': %d\n",filename,object->video->count);
#endif
	/** pCodecCtx->time_base has frame rate (struct with num/denom)! */
	/* setup config data */
	object->video->index = 0;
	object->video->flags |= VIDEO_FLAG_DO_UPDATE;
	object->video->flags |= VIDEO_FLAG_STEP;
	object->video->flags |= VIDEO_FLAG_IS_PAUSED;
}
/*----------------------------------------------------------------------------*/
void capture_live(my1video_capture_t* object, char *camname)
{
	/* check if captured... can we use this as captured flag? */
	if (object->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!object->video) return;
	/* try to initialize? */
	capture_core(object,camname);
	if (!object->fcontext) return;
	/* grab a 'sample' frame */
	if (capture_grab_frame(object))
	{
		capture_form_frame(object); /* get frame in rgb format */
		image_make(&object->video->image,
			object->video->height,object->video->width);
		/* capture_form_frame always convert rgb? */
		object->video->image.mask = IMASK_COLOR24;
		image_set_frame(&object->video->image,object->buffer);
		object->video->frame = &object->video->image;
	}
	/* create/init device */
	object->video->count = -1; /* already default! */
	object->video->index = -1;
	object->video->flags |= VIDEO_FLAG_DO_UPDATE;
	object->video->flags &= ~VIDEO_FLAG_STEP;
}
/*----------------------------------------------------------------------------*/
void capture_grab(my1video_capture_t* object)
{
	if (!object->fcontext) return;
	if (!object->video) return;
	object->video->flags &= ~VIDEO_FLAG_NEW_FRAME;
	object->ready = 0x0;
	if (!(object->video->flags&VIDEO_FLAG_DO_UPDATE)) return;
	if (object->video->count<0) /* flag for livefeed */
	{
		object->ready = capture_grab_frame(object);
	}
	else
	{
		/* check reset request */
		if (object->video->index==0)
		{
			capture_reset(object);
			object->index = 0;
		}
		/* compare index to see if grabbing is required */
		if (object->index<=object->video->index)
		{
			object->ready = capture_grab_frame(object);
			object->index++;
		}
	}
	if (object->ready) /* create video internal copy if valid frame */
	{
		capture_form_frame(object); /* get frame in rgb format */
		image_make(&object->video->image,
			object->video->height,object->video->width);
		/* capture_form_frame always convert rgb? */
		object->video->image.mask = IMASK_COLOR24;
		image_set_frame(&object->video->image,object->buffer);
		object->video->frame = &object->video->image;
		object->video->flags |= VIDEO_FLAG_NEW_FRAME;
	}
	if (object->video->flags&VIDEO_FLAG_STEP)
		object->video->flags &= ~VIDEO_FLAG_DO_UPDATE;
}
/*----------------------------------------------------------------------------*/
void capture_stop(my1video_capture_t* object)
{
	if (!object->fcontext) return;
	av_free(object->frame);
	object->frame = 0x0;
	av_free(object->buffer);
	object->buffer = 0x0;
	av_free(object->pixbuf);
	object->pixbuf = 0x0;
	sws_freeContext(object->rgb32fmt);
	object->rgb32fmt = 0x0;
	avcodec_close(object->ccontext);
	object->ccontext = 0x0;
	avformat_close_input(&object->fcontext);
	object->fcontext = 0x0;
}
/*----------------------------------------------------------------------------*/
void display_init(my1video_display_t* object, my1video_t* video)
{
	object->pixbuf = 0x0;
	object->video = video;
	image_view_init(&object->view);
	video->display = (void*) object;
}
/*----------------------------------------------------------------------------*/
void display_free(my1video_display_t* object)
{
	if (object->pixbuf)
		g_object_unref(object->pixbuf);
	image_view_free(&object->view);
}
/*----------------------------------------------------------------------------*/
void display_make(my1video_display_t* object)
{
	/* must have image frame */
	if (!object->video||!object->video->frame) return;
	image_view_make(&object->view,object->video->frame);
}
/*----------------------------------------------------------------------------*/
void display_draw(my1video_display_t* object)
{
	/* must have image frame */
	if (!object->video||!object->video->frame) return;
	image_view_draw(&object->view,object->video->frame);
}
/*----------------------------------------------------------------------------*/
void display_name(my1video_display_t* object,const char *name,const char *icon)
{
	if (!object->video) return;
	/* set title */
	image_view_name(&object->view,name);
	/* remove existing pixbuf */
	if (object->pixbuf)
		g_object_unref(object->pixbuf);
	/* read new? */
	if (icon)
	{
		object->pixbuf = gdk_pixbuf_new_from_file(icon,0x0);
		if (object->pixbuf)
			gtk_window_set_icon(GTK_WINDOW(object->view.window),object->pixbuf);
	}
}
/*----------------------------------------------------------------------------*/
