/*----------------------------------------------------------------------------*/
#include "my1libav_grab.h"
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
void libav1_init(my1libav_grab_t* vgrab, my1image_t* image)
{
	avdevice_register_all();
	vgrab->flag = 0x0;
	/* alloc stuff - init */
	vgrab->fcontext = 0x0;
	vgrab->ccontext = 0x0;
	vgrab->rgb32fmt = 0x0;
	vgrab->packet = 0x0;
	vgrab->pixbuf = 0x0;
	vgrab->buffer = 0x0;
	vgrab->frame = 0x0;
	/* alloc stuff - ends */
	vgrab->ready = 0x0;
	vgrab->image = image;
	vgrab->width = 0;
	vgrab->height = 0;
	vgrab->count = -1;
	vgrab->index = -1;
	vgrab->vstream = -1;
	vgrab->iloop = 1;
}
/*----------------------------------------------------------------------------*/
void libav1_free(my1libav_grab_t* vgrab)
{
	if (vgrab->frame)
	{
		av_free(vgrab->frame);
		vgrab->frame = 0x0;
	}
	if (vgrab->buffer)
	{
		av_free(vgrab->buffer);
		vgrab->buffer = 0x0;
	}
	if (vgrab->pixbuf)
	{
		av_free(vgrab->pixbuf);
		vgrab->pixbuf = 0x0;
	}
	if (vgrab->packet)
	{
		av_packet_free(&vgrab->packet);
		av_free(vgrab->packet);
		vgrab->packet = 0x0;
	}
	if (vgrab->rgb32fmt)
	{
		sws_freeContext(vgrab->rgb32fmt);
		vgrab->rgb32fmt = 0x0;
	}
	if (vgrab->ccontext)
	{
		avcodec_close(vgrab->ccontext);
		vgrab->ccontext = 0x0;
	}
	if (vgrab->fcontext)
	{
		avformat_close_input(&vgrab->fcontext);
		avformat_free_context(vgrab->fcontext);
		vgrab->fcontext = 0x0;
	}
	vgrab->flag = 0x0;
}
/*----------------------------------------------------------------------------*/
void* libav1_grab_frame(my1libav_grab_t* vgrab)
{
	int test, live = 0;
	AVFrame *frame = 0x0;
	AVCodecContext *pCodecCtx = vgrab->ccontext;
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	if (!vgrab->flag) return (void*)frame;
	while (av_read_frame(pFormatCtx,vgrab->packet)>=0)
	{
		/* look for video stream packet */
		if (vgrab->packet->stream_index==vgrab->vstream)
		{
			/** TODO: NEED TO REFINE THIS! */
			/* flush buffers for live feed */
			if (vgrab->count<0&&!live)
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
void libav1_reset(my1libav_grab_t* vgrab)
{
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	if (!pFormatCtx) return;
	if (avformat_seek_file(pFormatCtx, vgrab->vstream,
		0, vgrab->index, vgrab->count-1,
		AVSEEK_FLAG_ANY|AVSEEK_FLAG_FRAME)<0)
	{
		printf("error while seeking frame %d in '%s'!\n",
			vgrab->index,pFormatCtx->url);
	}
	avcodec_flush_buffers(vgrab->ccontext);
}
/*----------------------------------------------------------------------------*/
void libav1_from_frame(my1libav_grab_t* vgrab)
{
	/* convert to RGB! resize here as well? */
	sws_scale(vgrab->rgb32fmt,
		(const uint8_t**) vgrab->frame->data, vgrab->frame->linesize,
		0, vgrab->height, vgrab->buffer->data, vgrab->buffer->linesize);
}
/*----------------------------------------------------------------------------*/
void libav1_core(my1libav_grab_t* vgrab, char *doname)
{
	int loop, step, size;
	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec, *pCodecTemp;
	AVCodecParameters *pCodecParamFind, *pCodecParamTemp;
	if (vgrab->flag) return;
	/* allocate resource */
	pFormatCtx = avformat_alloc_context();
	if (!pFormatCtx)
	{
		printf("Cannot allocate context resource\n");
		return;
	}
	/* open resource - autodetect format & no dictionary */
	if (avformat_open_input(&pFormatCtx,doname,NULL,NULL))
	{
		avformat_free_context(pFormatCtx);
		printf("Cannot open resource %s\n",doname);
		return;
	}
	vgrab->fcontext = pFormatCtx;
	/* retrieve stream information - no dictionary */
	if (avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Cannot find stream info in resource %s\n",doname);
		return;
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
			continue;
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
		return;
	}
	/* success?! */
	vgrab->vstream = step;
	/* allocate memory for codec context */
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		printf("Cannot allocate memory for codec context\n");
		return;
	}
	vgrab->ccontext = pCodecCtx;
	if (avcodec_parameters_to_context(pCodecCtx, pCodecParamFind)<0)
	{
		printf("Cannot codec params to codec context\n");
		return;
	}
	/* prepare codec */
	if (avcodec_open2(pCodecCtx,pCodec,NULL)<0)
	{
		printf("Cannot open codec for file %s!\n",doname);
		exit(-1);
	}
	/* check size requirements */
	if (!vgrab->height) vgrab->height = pCodecCtx->height;
	if (!vgrab->width) vgrab->width = pCodecCtx->width;
	/* create RGB32 converter context */
	vgrab->rgb32fmt = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, vgrab->width, vgrab->height,
		AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	if (!vgrab->rgb32fmt)
	{
		printf("Cannot initialize the capture conversion context!\n");
		return;
	}
	/* allocate resource for decoding packet */
	vgrab->packet = av_packet_alloc();
	if (!vgrab->packet)
	{
		printf("Cannot allocate memory for packet!\n");
		return;
	}
	/* allocate video frames */
	vgrab->buffer = av_frame_alloc();
	if (!vgrab->buffer)
	{
		printf("Cannot allocate memory for rgb buffer frame!\n");
		return;
	}
	vgrab->frame = av_frame_alloc();
	if (!vgrab->frame)
	{
		printf("Cannot allocate memory for input frame!\n");
		return;
	}
	/* create RGB buffer space - 1-off */
	size = av_image_get_buffer_size(AV_PIX_FMT_RGB32,vgrab->width,
		vgrab->height,32); /* byte alignment! */
	vgrab->pixbuf = (uint8_t *)av_malloc(size*sizeof(uint8_t));
	av_image_fill_arrays(vgrab->buffer->data,vgrab->buffer->linesize,
		vgrab->pixbuf,AV_PIX_FMT_RGB32,vgrab->width,vgrab->height,1);
	if (!vgrab->pixbuf)
	{
		printf("Cannot allocate memory for pixbuf!\n");
		return;
	}
	vgrab->flag = vgrab->fcontext;
}
/*----------------------------------------------------------------------------*/
void libav1_file(my1libav_grab_t* vgrab, char *filename)
{
	libav1_core(vgrab,filename);
	if (!vgrab->flag) return;
	/* grab a sample */
	if (libav1_grab_frame(vgrab))
	{
		libav1_from_frame(vgrab);
		image_make(vgrab->image,vgrab->height,vgrab->width);
		vgrab->image->mask = IMASK_COLOR;
		image_from_frame(vgrab->image,vgrab->buffer);
		/* count frames - already grabbed one! */
		vgrab->count = 1;
		while (libav1_grab_frame(vgrab)) vgrab->count++;
		/** pCodecCtx->time_base has frame rate (struct with num/denom)! */
		vgrab->index = 0;
		libav1_reset(vgrab);
		vgrab->ready = vgrab->frame;
	}
}
/*----------------------------------------------------------------------------*/
void libav1_live(my1libav_grab_t* vgrab, char *camname)
{
	libav1_core(vgrab,camname);
	if (!vgrab->flag) return;
	/* grab a sample */
	if (libav1_grab_frame(vgrab))
	{
		libav1_from_frame(vgrab);
		image_make(vgrab->image,vgrab->height,vgrab->width);
		vgrab->image->mask = IMASK_COLOR;
		image_from_frame(vgrab->image,vgrab->buffer);
		vgrab->ready = vgrab->frame;
	}
}
/*----------------------------------------------------------------------------*/
void libav1_grab(my1libav_grab_t* vgrab, my1image_t* image)
{
	if (!vgrab->flag) return;
	vgrab->ready = libav1_grab_frame(vgrab);
	if (vgrab->ready)
	{
		if (vgrab->count>0)
		{
			vgrab->index++;
			if (vgrab->iloop)
			{
				if (vgrab->index==vgrab->count)
				{
					vgrab->index = 0;
					libav1_reset(vgrab);
				}
			}
			/* else, upper level will handle this! */
		}
		libav1_from_frame(vgrab);
		image_make(image,vgrab->height,vgrab->width);
		image->mask = IMASK_COLOR;
		image_from_frame(image,vgrab->buffer);
	}
}
/*----------------------------------------------------------------------------*/
