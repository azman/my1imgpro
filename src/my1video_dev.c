/*----------------------------------------------------------------------------*/
#include "my1video_dev.h"
/*----------------------------------------------------------------------------*/
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
/*----------------------------------------------------------------------------*/
#include <gdk/gdkkeysyms.h>
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
void capture_init(my1video_capture_t* vgrab, my1video_t* video)
{
	avcodec_register_all();
	av_register_all();
	avdevice_register_all();
	vgrab->fcontext = 0x0;
	vgrab->vstream = -1;
	vgrab->ccontext = 0x0;
	vgrab->rgb32fmt = 0x0;
	vgrab->pixbuf = 0x0;
	vgrab->strbuf = (uint8_t*)
		malloc(STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	memset((void*)vgrab->strbuf,0,STRBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE);
	vgrab->packet = av_packet_alloc();
	vgrab->frame = 0x0;
	vgrab->buffer = 0x0;
	vgrab->ready = 0x0;
	vgrab->video = video;
	vgrab->index = -1;
	video->capture = (void*) vgrab;
}
/*----------------------------------------------------------------------------*/
void capture_free(my1video_capture_t* vgrab)
{
	if (vgrab->frame) av_free(vgrab->frame);
	if (vgrab->buffer) av_free(vgrab->buffer);
	if (vgrab->rgb32fmt) sws_freeContext(vgrab->rgb32fmt);
	if (vgrab->pixbuf) av_free(vgrab->pixbuf);
	if (vgrab->ccontext) avcodec_close(vgrab->ccontext);
	if (vgrab->fcontext) avformat_close_input(&vgrab->fcontext);
}
/*----------------------------------------------------------------------------*/
void* capture_grab_frame(my1video_capture_t* vgrab)
{
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	AVCodecContext *pCodecCtx = vgrab->ccontext;
	AVFrame *frame = 0x0;
	AVPacket packet;
	int test;
	if (!pFormatCtx) return frame;
	while (av_read_frame(pFormatCtx,&packet)>=0)
	{
		/* look for video stream packet */
		if (packet.stream_index==vgrab->vstream)
		{
			/* decode video frame - NEW! */
			test = avcodec_send_packet(pCodecCtx,&packet);
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
void capture_reset(my1video_capture_t* vgrab)
{
	AVFormatContext *pFormatCtx = vgrab->fcontext;
	if (!pFormatCtx) return;
	if (avformat_seek_file(pFormatCtx, vgrab->vstream,
		0, vgrab->video->index, vgrab->video->count-1,
		AVSEEK_FLAG_ANY|AVSEEK_FLAG_FRAME)<0)
	{
		printf("error while seeking frame %d in '%s'!\n",
			vgrab->video->index,pFormatCtx->filename);
		exit(-1);
	}
	avcodec_flush_buffers(vgrab->ccontext);
}
/*----------------------------------------------------------------------------*/
void capture_form_frame(my1video_capture_t* vgrab)
{
	/* convert to RGB! resize here as well? */
	sws_scale(vgrab->rgb32fmt,
		(const uint8_t**) vgrab->frame->data, vgrab->frame->linesize,
		0, vgrab->video->height,
		vgrab->buffer->data, vgrab->buffer->linesize);
}
/*----------------------------------------------------------------------------*/
void capture_core(my1video_capture_t* vgrab, char *doname)
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
	vgrab->fcontext = pFormatCtx;
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
	vgrab->vstream = loop;
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
		printf("Cannot allocate memory for video frame!\n");
		exit(-1);
	}
	/* create RGB buffer - allocate the actual pixel buffer */
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
		capture_form_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_form_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR24;
		image_set_frame(&vgrab->video->image,vgrab->buffer);
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
		capture_form_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_form_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR24;
		image_set_frame(&vgrab->video->image,vgrab->buffer);
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
		capture_form_frame(vgrab); /* get frame in rgb format */
		image_make(&vgrab->video->image,
			vgrab->video->height,vgrab->video->width);
		/* capture_form_frame always convert rgb? */
		vgrab->video->image.mask = IMASK_COLOR24;
		image_set_frame(&vgrab->video->image,vgrab->buffer);
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
	av_free(vgrab->buffer);
	vgrab->buffer = 0x0;
	av_free(vgrab->pixbuf);
	vgrab->pixbuf = 0x0;
	sws_freeContext(vgrab->rgb32fmt);
	vgrab->rgb32fmt = 0x0;
	avcodec_close(vgrab->ccontext);
	vgrab->ccontext = 0x0;
	avformat_close_input(&vgrab->fcontext);
	vgrab->fcontext = 0x0;
}
/*----------------------------------------------------------------------------*/
void display_init(my1video_display_t* vview, my1video_t* video)
{
	vview->pixbuf = 0x0;
	vview->timer = 0;
	vview->delms = FRAME_NEXT_MS;
	vview->chkinput = 0x0;
	vview->chkinput_data = 0x0;
	vview->video = video;
	image_view_init(&vview->view);
	video->display = (void*) vview;
}
/*----------------------------------------------------------------------------*/
void display_free(my1video_display_t* vview)
{
	if (vview->timer)
		g_source_remove(vview->timer);
	if (vview->pixbuf)
		g_object_unref(vview->pixbuf);
	image_view_free(&vview->view);
}
/*----------------------------------------------------------------------------*/
gint on_display_key_press(GtkWidget *widget, GdkEventKey *kevent, gpointer data)
{
	if(kevent->type == GDK_KEY_PRESS)
	{
		my1video_t *video = (my1video_t*) data;
		/** g_message("%d, %c", kevent->keyval, kevent->keyval); */
		video->inkey = (int) kevent->keyval;
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void display_make(my1video_display_t* vview)
{
	/* must have image frame */
	if (!vview->video||!vview->video->frame) return;
	image_view_make(&vview->view,vview->video->frame);
	/* add default control? */
	g_signal_connect(G_OBJECT(vview->view.window),"key_press_event",
		G_CALLBACK(on_display_key_press),(gpointer)vview->video);
}
/*----------------------------------------------------------------------------*/
gboolean on_display_timer(gpointer data)
{
	my1video_t* video = (my1video_t*) data;
	my1video_capture_t* vgrab = (my1video_capture_t*)video->capture;
	my1video_display_t* vview = (my1video_display_t*)video->display;
	guint keyval = (guint)video->inkey;
	capture_grab(vgrab);
	if(video->flags&VIDEO_FLAG_NEW_FRAME)
	{
		video_post_frame(video);
		video_filter_frame(video);
		display_draw((my1video_display_t*)video->display);
	}
	if(keyval == GDK_KEY_Escape||
		keyval == GDK_KEY_q || vview->view.doquit)
	{
		gtk_main_quit();
	}
	else if(keyval == GDK_KEY_c)
	{
		video_play(video);
		image_view_stat_time(&vview->view,"Play",MESG_SHOWTIME);
	}
	else if(keyval == GDK_KEY_s)
	{
		video_stop(video);
		image_view_stat_time(&vview->view,"Stop",MESG_SHOWTIME);
	}
	else if(keyval == GDK_KEY_space)
	{
		if (video->index!=video->count||video->flags&VIDEO_FLAG_LOOP)
		{
			video_hold(video);
			if (video->flags&VIDEO_FLAG_IS_PAUSED)
				image_view_stat_time(&vview->view,"Paused",MESG_SHOWTIME);
			else
				image_view_stat_time(&vview->view,"Play",MESG_SHOWTIME);
		}
	}
	else if(keyval == GDK_KEY_f)
	{
		video_next_frame(video);
		image_view_stat_time(&vview->view,"Next",MESG_SHOWTIME);
	}
	else if(keyval == GDK_KEY_b)
	{
		if (video->count<0)
		{
			image_view_stat_time(&vview->view,
				"Not during live feed!",MESG_SHOWTIME);
		}
		else
		{
			video_prev_frame(video);
			image_view_stat_time(&vview->view,"Previous",MESG_SHOWTIME);
		}
	}
	else if(keyval == GDK_KEY_l)
	{
		video_loop(video);
		if (video->flags&VIDEO_FLAG_LOOP)
			image_view_stat_time(&vview->view,"Looping ON",MESG_SHOWTIME);
		else
			image_view_stat_time(&vview->view,"Looping OFF",MESG_SHOWTIME);
	}
	else if(keyval == GDK_KEY_z)
	{
		video_skip_filter(video);
		if (video->flags&VIDEO_FLAG_NO_FILTER)
			image_view_stat_time(&vview->view,"Filter ON",MESG_SHOWTIME);
		else
			image_view_stat_time(&vview->view,"Filter OFF",MESG_SHOWTIME);
	}
	else
	{
		if (vview->chkinput)
			vview->chkinput((void*)vview);
	}
	video->inkey = 0;
	video_post_input(video);
	return TRUE; /* return FALSE to stop */
}
/*----------------------------------------------------------------------------*/
void display_loop(my1video_display_t* vview, int delms)
{
	if (delms>0) vview->delms = delms;
	/* ignore if one is running */
	if (vview->timer) return;
	vview->timer = g_timeout_add(vview->delms,on_display_timer,
		(gpointer)vview->video);
}
/*----------------------------------------------------------------------------*/
void display_draw(my1video_display_t* vview)
{
	/* must have image frame */
	if (!vview->video||!vview->video->frame) return;
	image_view_draw(&vview->view,vview->video->frame);
}
/*----------------------------------------------------------------------------*/
void display_name(my1video_display_t* vview,const char *name,const char *icon)
{
	if (!vview->video) return;
	/* set title */
	image_view_name(&vview->view,name);
	/* remove existing pixbuf */
	if (vview->pixbuf)
		g_object_unref(vview->pixbuf);
	/* read new? */
	if (icon)
	{
		vview->pixbuf = gdk_pixbuf_new_from_file(icon,0x0);
		if (vview->pixbuf)
			gtk_window_set_icon(GTK_WINDOW(vview->view.window),vview->pixbuf);
	}
}
/*----------------------------------------------------------------------------*/
