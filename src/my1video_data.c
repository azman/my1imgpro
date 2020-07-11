/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_MAINC__
#define __MY1VIDEO_MAINC__
/*----------------------------------------------------------------------------*/
#include "my1video_data.h"
#include "my1image_main.h"
#include "my1image_crgb.h"
#include "my1image_ifdb.h"
/*----------------------------------------------------------------------------*/
/** need this to save a frame from video to file! */
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
int video_data_init(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1vdata_t *what = (my1vdata_t*)task->data;
	my1imain_t *main = (my1imain_t*)that;
	libav1_init(&what->vgrab,&main->load);
	what->vgrab.iloop = 0; /* no need to check index, do it here! */
	image_appw_init(&what->vappw);
	video_init(&what->video);
	what->flag = VIDEO_SOURCE_NONE;
	return 0;
}
/*----------------------------------------------------------------------------*/
int video_data_free(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1vdata_t *what = (my1vdata_t*)task->data;
	video_free(&what->video);
	image_appw_free(&what->vappw);
	libav1_free(&what->vgrab);
	return 0;
}
/*----------------------------------------------------------------------------*/
void video_data_grab(my1vdata_t* vmain, my1imain_t* imain)
{
	int grab = 0;
	my1igrab_t* igrab = (my1igrab_t*) &imain->grab;
	my1video_t* video = (my1video_t*) &vmain->video;
	my1vgrab_t* vgrab = (my1vgrab_t*) &vmain->vgrab;
	/* video stuff */
	video->flags &= ~VIDEO_FLAG_NEW_FRAME;
	vgrab->ready = 0x0;
	if (!(video->flags&VIDEO_FLAG_DO_UPDATE)) return;
	if (vgrab->count<0) grab = 1; /* live ALWAYS grab */
	else /* check the need to grab */
	{
		if (video->index==0)
		{
			vgrab->index = 0;
			libav1_reset(vgrab);
		}
		else if (video->index<vgrab->index)
		{
			vgrab->index = video->index;
			libav1_reset(vgrab);
		}
		if (vgrab->index<=video->index) grab = 1;
	}
	if (grab)
	{
		libav1_grab(vgrab,igrab->grab);
	}
	if (vgrab->ready)
	{
		video->frame = &video->image;
		video->flags |= VIDEO_FLAG_NEW_FRAME;
	}
	if (video->flags&VIDEO_FLAG_STEP)
		video->flags &= ~VIDEO_FLAG_DO_UPDATE;
	/* the old display on timer processing */
	if (video->flags&VIDEO_FLAG_NEW_FRAME)
		video_post_frame(video);
}
/*----------------------------------------------------------------------------*/
int igrab_grab_video(void* data, void* that, void* xtra)
{
	my1itask_t* ptemp = (my1itask_t*) data;
	my1image_grab_t* igrab = (my1image_grab_t*) that;
	my1vdata_t *what = (my1vdata_t*)ptemp->data;
	my1imain_t *main = (my1imain_t*)ptemp->temp;
	my1libav_grab_t* vgrab = &what->vgrab;
	if (!vgrab->flag)
	{
		if (what->flag==VIDEO_SOURCE_LIVE)
			libav1_live(vgrab,igrab->pick);
		else /**if (what->flag==VIDEO_SOURCE_FILE)*/
			libav1_file(vgrab,igrab->pick);
		if (!vgrab->flag)
		{
			igrab->flag |= IGRAB_FLAG_LOAD_ERROR;
			return igrab->flag;
		}
		else
		{
			what->video.index = 0;
			what->video.count = vgrab->count;
			if (vgrab->count>0)
				what->video.flags |= VIDEO_FLAG_IS_PAUSED;
			what->video.flags |= VIDEO_FLAG_DO_UPDATE;
		}
	}
	video_data_grab(what,main);
	return igrab->flag;
}
/*----------------------------------------------------------------------------*/
int video_data_args(void* data, void* that, void* xtra)
{
	int loop, argc, *temp = (int*) that;
	char** argv = (char**) xtra;
	my1itask_t *task = (my1itask_t*)data;
	my1vdata_t *what = (my1vdata_t*)task->data;
	my1imain_t *main = (my1imain_t*)task->temp;
	my1igrab_t *grab = (my1igrab_t*)&main->grab;
	argc = *temp;
	if (main->flag&IFLAG_ERROR) return 0;
	/* re-check parameter for video option */
	loop = 1;
	what->flag = VIDEO_SOURCE_FILE; /* assume pick is video */
	if (argc>2)
	{
		if (!strncmp(grab->pick,"--live",7))
		{
			grab->pick = argv[++loop];
			what->flag = VIDEO_SOURCE_LIVE;
		}
	}
	main->flag |= IFLAG_VIDEO_MODE; /* mark for video display menu */
	grab->do_grab.task = igrab_grab_video;
	grab->do_grab.data = (void*)what;
	grab->do_grab.temp = (void*)main;
	grab->grab = &main->load;
	/* save these */
	what->loop = loop;
	what->argc = argc;
	what->argv = argv;
	return 0;
}
/*----------------------------------------------------------------------------*/
int video_data_prep(void* data, void* that, void* xtra)
{
	my1imain_t *main = (my1imain_t*)that;
	main->list = image_work_create_all();
	return 0;
}
/*----------------------------------------------------------------------------*/
int video_data_exec(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1vdata_t *what = (my1vdata_t*)task->data;
	my1imain_t *main = (my1imain_t*)that;
	image_copy(&what->video.image,main->show);
	what->video.frame = &what->video.image;
	main->show = &what->video.image;
	return 0;
}
/*----------------------------------------------------------------------------*/
int video_data_draw_index(void* data, void* that, void* xtra)
{
	my1image_view_t* view = (my1image_view_t*) that;
	my1itask_t *itask = (my1itask_t*)data;
	my1vdata_t* vmain = (my1vdata_t*) itask->data;
	my1video_t* video = &vmain->video;
	my1vgrab_t* vgrab = &vmain->vgrab;
	gchar *buff = 0x0;
	if (video->count<0) return 0; /* live? */
	cairo_set_source_rgb(view->dodraw,0.0,0.0,1.0);
	cairo_move_to(view->dodraw,20,20);
	cairo_set_font_size(view->dodraw,12);
	buff = g_strdup_printf("%d/%d(%d)",video->index,video->count,vgrab->index);
	cairo_show_text(view->dodraw,buff);
	cairo_stroke(view->dodraw);
	g_free(buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
int vmain_on_keychk(void* data, void* that, void* xtra)
{
	my1itask_t *itask = (my1itask_t*)data;
	my1ishow_t *vappw = (my1ishow_t*) that;
	my1vdata_t *vmain = (my1vdata_t*) itask->data;
	my1imain_t *imain = (my1imain_t*) itask->temp;
	my1video_t *video = (my1video_t*) &vmain->video;
	GdkEventKey *event = (GdkEventKey*) xtra;
	guint keyval = event->keyval;
	if (keyval == GDK_KEY_c)
	{
		video_play(video);
		image_appw_stat_time(vappw,"Play",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_s)
	{
		video_stop(video);
		image_appw_stat_time(vappw,"Stop",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_space)
	{
		if (video->index!=video->count||video->flags&VIDEO_FLAG_LOOP)
		{
			if (video->flags&VIDEO_FLAG_IS_PAUSED)
			{
				video_hold(video,0);
				image_appw_stat_time(vappw,"Play",MESG_SHOWTIME);
			}
			else
			{
				video_hold(video,1);
				image_appw_stat_time(vappw,"Paused",MESG_SHOWTIME);
			}
		}
	}
	else if (keyval == GDK_KEY_bracketright)
	{
		video_next_frame(video);
		image_appw_stat_time(vappw,"Next",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_bracketleft)
	{
		if (video->count<0)
		{
			image_appw_stat_time(vappw,"Not during live feed!",MESG_SHOWTIME);
		}
		else
		{
			video_prev_frame(video);
			image_appw_stat_time(vappw,"Previous",MESG_SHOWTIME);
		}
	}
	else if (keyval == GDK_KEY_l)
	{
		if (video->flags&VIDEO_FLAG_LOOP)
		{
			video_loop(video,0);
			image_appw_stat_time(vappw,"Looping OFF",MESG_SHOWTIME);
		}
		else
		{
			video_loop(video,1);
			image_appw_stat_time(vappw,"Looping ON",MESG_SHOWTIME);
		}
	}
	else if (keyval == GDK_KEY_g)
	{
		GtkWidget *dosave;
		/* stop video on grabbing */
		video_stop(video);
		/* create save dialog */
		dosave = gtk_file_chooser_dialog_new("Save Image File",
			GTK_WINDOW(vappw->window),GTK_FILE_CHOOSER_ACTION_SAVE,
			"_Cancel", GTK_RESPONSE_CANCEL,
			"_Open", GTK_RESPONSE_ACCEPT, NULL);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dosave),
			TRUE);
		/* show it! */
		if (gtk_dialog_run(GTK_DIALOG(dosave))==GTK_RESPONSE_ACCEPT)
		{
			gchar *filename;
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dosave));
			if (image_save(video->frame,filename))
				image_appw_stat_time(vappw,"Grab failed!",MESG_SHOWTIME);
			else
				image_appw_stat_time(vappw,"Frame saved!",MESG_SHOWTIME);
			g_free(filename);
		}
		gtk_widget_destroy(dosave);
	}
	else if (keyval == GDK_KEY_z)
	{
		imain_menu_filter_enable(imain,!(imain->flag&IFLAG_FILTER_EXE));
	}
	video_post_input(video);
	return 0;
}
/*----------------------------------------------------------------------------*/
int video_data_show(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1vdata_t *what = (my1vdata_t*)task->data;
	my1imain_t *main = (my1imain_t*)that;
	/* modify name for main win */
	image_appw_name(&main->iwin,"MY1Video Viewer");
	imain_domenu_filters(main);
	image_appw_domenu_quit(&main->iwin);
	itask_make(&main->iwin.view.domore,video_data_draw_index,(void*)what);
	itask_make(&main->iwin.keychk,vmain_on_keychk,(void*)what);
	main->iwin.keychk.temp = (void*) main;
	imain_loop(main,0); /* control tdel? */
	return 0;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_MAINC__ */
/*----------------------------------------------------------------------------*/
