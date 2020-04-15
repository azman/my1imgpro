/*----------------------------------------------------------------------------*/
#include "my1video_grab.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
/*----------------------------------------------------------------------------*/
void image_from_frame(my1image_t* image, AVFrame* frame)
{
	int loop, step, *pdat;
	my1rgb_t temp;
	temp.a = 0; /* alpha always zero? */
	for (loop=0,step=0;loop<image->size;loop++)
	{
		temp.b = frame->data[0][step++];
		temp.g = frame->data[0][step++];
		temp.r = frame->data[0][step++];
		step++; /* ignore alpha */
		pdat = (int*)&temp;
		image->data[loop] = *pdat;
	}
}
/*----------------------------------------------------------------------------*/
void capture_init(my1video_capture_t* vgrab, my1video_t* video)
{
	avdevice_register_all();
	vgrab->fcontext = 0x0;
	vgrab->ccontext = 0x0;
	vgrab->rgb32fmt = 0x0;
	vgrab->packet = 0x0;
	vgrab->pixbuf = 0x0;
	vgrab->buffer = 0x0;
	vgrab->frame = 0x0;
	vgrab->ready = 0x0;
	vgrab->video = video;
	vgrab->index = -1;
	vgrab->vstream = -1;
	video->capture = (void*) vgrab;
}
/*----------------------------------------------------------------------------*/
void capture_free(my1video_capture_t* vgrab)
{
	if (vgrab->frame) av_free(vgrab->frame);
	if (vgrab->pixbuf) av_free(vgrab->pixbuf);
	if (vgrab->buffer) av_free(vgrab->buffer);
	if (vgrab->rgb32fmt) sws_freeContext(vgrab->rgb32fmt);
	if (vgrab->packet) av_packet_free(&vgrab->packet);
	if (vgrab->ccontext) avcodec_close(vgrab->ccontext);
	if (vgrab->fcontext) avformat_close_input(&vgrab->fcontext);
}
/*----------------------------------------------------------------------------*/
void* capture_grab_frame(my1video_capture_t* vgrab)
{
	int test, live = 0;
	AVFrame *frame = 0x0;
	AVCodecContext *pCodecCtx = vgrab->ccontext;
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	if (!pFormatCtx) return (void*)frame;
	while (av_read_frame(pFormatCtx,vgrab->packet)>=0)
	{
		/* look for video stream packet */
		if (vgrab->packet->stream_index==vgrab->vstream)
		{
			/** TODO: NEED TO REFINE THIS! */
			/* flush buffers for live feed */
			if (vgrab->video->count<0&&!live)
			{
				/* enter draining mode */
				avcodec_send_packet(pCodecCtx,NULL);
				while (avcodec_receive_frame(pCodecCtx,vgrab->frame)>=0);
				/* reset codec to resume decoding */
				avcodec_flush_buffers(pCodecCtx);
				live = 1;
				continue;
			}
			/* decode video frame - NEW! */
			test = avcodec_send_packet(pCodecCtx,vgrab->packet);
			if (test>=0)
			{
				test = avcodec_receive_frame(pCodecCtx,vgrab->frame);
				if (test>=0) /* 0: success */
				{
					/* did we get a complete frame? */
					frame = vgrab->frame;
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
		/* free the packet from av_read_frame */
		av_packet_unref(vgrab->packet);
		if (frame) break;
	}
	return frame;
}
/**
 * to drain codec internal buffers:
 * - avcodec_send_packet(NULL,NULL); => enter draining mode
 * - avcodec_receive_frame(); => loop until AVERROR_EOF returned
 * to reset codec to resume decoding:
 * - avcodec_flush_buffers();
 *
**/
/*----------------------------------------------------------------------------*/
void capture_reset(my1video_capture_t* vgrab)
{
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	if (!pFormatCtx) return;
	if (avformat_seek_file(pFormatCtx, vgrab->vstream,
		0, vgrab->video->index, vgrab->video->count-1,
		AVSEEK_FLAG_ANY|AVSEEK_FLAG_FRAME)<0)
	{
		printf("error while seeking frame %d in '%s'!\n",
			vgrab->video->index,pFormatCtx->url);
		exit(-1);
	}
	avcodec_flush_buffers(vgrab->ccontext);
}
/*----------------------------------------------------------------------------*/
void capture_from_frame(my1video_capture_t* vgrab)
{
	/* convert to RGB! resize here as well? */
	sws_scale(vgrab->rgb32fmt,
		(const uint8_t**) vgrab->frame->data, vgrab->frame->linesize,
		0, vgrab->video->height, vgrab->buffer->data, vgrab->buffer->linesize);
}
/*----------------------------------------------------------------------------*/
void capture_core(my1video_capture_t* vgrab, char *doname)
{
	int loop, step, size;
	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec, *pCodecTemp;
	AVCodecParameters *pCodecParamFind, *pCodecParamTemp;
	/* allocate resource */
	pFormatCtx = avformat_alloc_context();
	if (!pFormatCtx)
	{
		printf("Cannot allocate context resource\n");
		exit(-1);
	}
	/* open resource - autodetect format & no dictionary */
	if (avformat_open_input(&pFormatCtx,doname,NULL,NULL))
	{
		printf("Cannot open resource %s\n",doname);
		exit(-1);
	}
	vgrab->fcontext = pFormatCtx;
	/* retrieve stream information - no dictionary */
	if (avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Cannot find stream info in resource %s\n",doname);
		exit(-1);
	}
	pCodec = 0x0;
	pCodecParamFind =  0x0;
	step = -1;
	/** loop though all the streams and find video stream */
	for (loop=0;loop<pFormatCtx->nb_streams;loop++)
	{
		pCodecParamTemp = pFormatCtx->streams[loop]->codecpar;
		pCodecTemp = avcodec_find_decoder(pCodecParamTemp->codec_id);
		if (!pCodecTemp)
		{
			printf("Unsupported codec\n");
			exit(-1);
		}
		if (pCodecParamTemp->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			if (step<0)
			{
				step = loop;
				pCodec = pCodecTemp;
				pCodecParamFind = pCodecParamTemp;
			}
			/** res : pCodecParamTemp->width x pCodecParamTemp->height */
		}
		else if (pCodecParamTemp->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			/** pCodecParamTemp->channels , pCodecParamTemp->sample_rate */
		}
		/** pCodecTemp->name pCodecTemp->id pCodecTemp->bit_rate */
	}
	if (!pCodec)
	{
		printf("Unsupported codec???\n");
		exit(-1);
	}
	/* success?! */
	vgrab->vstream = step;
	/* allocate memory for codec context */
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		printf("Cannot allocate memory for codec context\n");
		exit(-1);
	}
	if (avcodec_parameters_to_context(pCodecCtx, pCodecParamFind)<0)
	{
		printf("Cannot codec params to codec context\n");
		exit(-1);
	}
	vgrab->ccontext = pCodecCtx;
	/* prepare codec */
	if (avcodec_open2(pCodecCtx,pCodec,NULL)<0)
	{
		printf("Cannot open codec for file %s!\n",doname);
		exit(-1);
	}
	/* check size requirements */
	if (!vgrab->video->height) vgrab->video->height = pCodecCtx->height;
	if (!vgrab->video->width) vgrab->video->width = pCodecCtx->width;
	/* create RGB32 converter context */
	vgrab->rgb32fmt = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, vgrab->video->width, vgrab->video->height,
		AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	if (vgrab->rgb32fmt==0x0)
	{
		printf("Cannot initialize the capture conversion context!\n");
		exit(-1);
	}
	/* allocate video frames */
	vgrab->frame = av_frame_alloc();
	if (vgrab->frame==0x0)
	{
		printf("Cannot allocate memory for input frame!\n");
		exit(-1);
	}
	vgrab->buffer = av_frame_alloc();
	if (vgrab->buffer==0x0)
	{
		printf("Cannot allocate memory for rgb buffer frame!\n");
		exit(-1);
	}
	/* allocate resource for decoding packet */
	vgrab->packet = av_packet_alloc();
	if (vgrab->packet==0x0)
	{
		printf("Cannot allocate memory for packet!\n");
		exit(-1);
	}
	/* create RGB buffer space - 1-off */
	size = av_image_get_buffer_size(AV_PIX_FMT_RGB32,vgrab->video->width,
		vgrab->video->height,32); /* byte alignment! */
	vgrab->pixbuf = (uint8_t *)av_malloc(size*sizeof(uint8_t));
	av_image_fill_arrays(vgrab->buffer->data,vgrab->buffer->linesize,
		vgrab->pixbuf,AV_PIX_FMT_RGB32,
		vgrab->video->width,vgrab->video->height,1);
}
/*----------------------------------------------------------------------------*/
void capture_file(my1video_capture_t* vgrab, char *filename)
{
	/* check if captured... can we use this as captured flag? */
	if (vgrab->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!vgrab->video) return;
	/* try to initialize? */
	capture_core(vgrab,filename);
	if (!vgrab->fcontext) return;
	/* count frame? */
	vgrab->video->count = 0;
	/* grab a 'sample' frame */
	if (capture_grab_frame(vgrab))
	{
		capture_from_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_from_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR;
		image_from_frame(&vgrab->video->image,vgrab->buffer);
		vgrab->video->frame = &vgrab->video->image;
		do
		{
			vgrab->video->count++;
		}
		while (capture_grab_frame(vgrab));
	}
	else
	{
		printf("Cannot get frames from '%s'?\n",filename);
		exit(-1);
	}
#ifdef MY1DEBUG
	printf("Frame Count for '%s': %d\n",filename,vgrab->video->count);
#endif
	/** pCodecCtx->time_base has frame rate (struct with num/denom)! */
	/* setup config data */
	vgrab->video->index = 0;
	vgrab->video->flags |= VIDEO_FLAG_DO_UPDATE;
	vgrab->video->flags |= VIDEO_FLAG_STEP;
	vgrab->video->flags |= VIDEO_FLAG_IS_PAUSED;
}
/*----------------------------------------------------------------------------*/
void capture_live(my1video_capture_t* vgrab, char *camname)
{
	/* check if captured... can we use this as captured flag? */
	if (vgrab->fcontext) return;
	/* abort if my1Video NOT defined! */
	if (!vgrab->video) return;
	/* try to initialize? */
	capture_core(vgrab,camname);
	if (!vgrab->fcontext) return;
	/* grab a 'sample' frame */
	if (capture_grab_frame(vgrab))
	{
		capture_from_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_from_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR;
		image_from_frame(&vgrab->video->image,vgrab->buffer);
		vgrab->video->frame = &vgrab->video->image;
	}
	/* create/init device */
	vgrab->video->count = -1; /* already default! */
	vgrab->video->index = -1;
	vgrab->video->flags |= VIDEO_FLAG_DO_UPDATE;
	vgrab->video->flags &= ~VIDEO_FLAG_STEP;
}
/*----------------------------------------------------------------------------*/
void capture_grab(my1video_capture_t* vgrab)
{
	if (!vgrab->fcontext) return;
	if (!vgrab->video) return;
	vgrab->video->flags &= ~VIDEO_FLAG_NEW_FRAME;
	vgrab->ready = 0x0;
	if (!(vgrab->video->flags&VIDEO_FLAG_DO_UPDATE)) return;
	if (vgrab->video->count<0) /* flag for livefeed */
	{
		vgrab->ready = capture_grab_frame(vgrab);
	}
	else
	{
		/* check reset request */
		if (vgrab->video->index==0)
		{
			capture_reset(vgrab);
			vgrab->index = 0;
		}
		else if (vgrab->video->index<vgrab->index)
		{
			/* try to reverse step? */
			capture_reset(vgrab);
			vgrab->index = vgrab->video->index;
		}
		/* compare index to see if grabbing is required */
		if (vgrab->index<=vgrab->video->index)
		{
			vgrab->ready = capture_grab_frame(vgrab);
			vgrab->index++;
		}
	}
	if (vgrab->ready) /* create video internal copy if valid frame */
	{
		capture_from_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_from_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR;
		image_from_frame(&vgrab->video->image,vgrab->buffer);
		vgrab->video->frame = &vgrab->video->image;
		vgrab->video->flags |= VIDEO_FLAG_NEW_FRAME;
	}
	if (vgrab->video->flags&VIDEO_FLAG_STEP)
		vgrab->video->flags &= ~VIDEO_FLAG_DO_UPDATE;
}
/*----------------------------------------------------------------------------*/
void capture_stop(my1video_capture_t* vgrab)
{
	if (!vgrab->fcontext) return;
	av_free(vgrab->frame);
	vgrab->frame = 0x0;
	av_packet_free(&vgrab->packet);
	vgrab->packet = 0x0;
	avcodec_close(vgrab->ccontext);
	vgrab->ccontext = 0x0;
	avformat_close_input(&vgrab->fcontext);
	vgrab->fcontext = 0x0;
}
/*----------------------------------------------------------------------------*/
