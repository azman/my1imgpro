/*----------------------------------------------------------------------------*/
#include "my1video_dev.h"
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1video_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "Basic Video Tool Library"
#endif
/*----------------------------------------------------------------------------*/
static char showkeys[] =
{
	"\t---------------------------\n"
	"\tDefined Keys:\n"
	"\t---------------------------\n"
	"\tc       - play video\n"
	"\ts       - stop video\n"
	"\t<SPACE> - pause video\n"
	"\tf       - next frame\n"
	"\tb       - previous frame\n"
	"\tl       - toggle looping\n"
	"\th       - show this message\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
#define FRAME_NEXT_MS 10
#define MESG_SHOWTIME 1
/*----------------------------------------------------------------------------*/
void video_draw_index(void* data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	my1video_t* video = (my1video_t*)view->draw_more_data;
	my1video_capture_t* vgrab = (my1video_capture_t*)video->capture;
	gchar *buff = 0x0;
	if (video->count<0) return;
	cairo_set_source_rgb(view->dodraw,0.0,0.0,1.0);
	cairo_move_to(view->dodraw,20,20);
	cairo_set_font_size(view->dodraw,12);
	buff = g_strdup_printf("%d/%d(%d)",
		video->index, video->count, vgrab->index);
	cairo_show_text(view->dodraw,buff);
	cairo_stroke(view->dodraw);
	g_free(buff);
}
/*----------------------------------------------------------------------------*/
gboolean on_timer_callback(gpointer data)
{
	my1video_t* video = (my1video_t*) data;
	my1video_capture_t* vgrab = (my1video_capture_t*)video->capture;
	my1video_display_t* vview = (my1video_display_t*)video->display;
	guint keyval = (guint)video->inkey;
	capture_grab(vgrab);
	if(video->flags&VIDEO_FLAG_NEW_FRAME)
	{
		video_post_frame(video);
		video_filter(video);
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
				image_view_stat_time(&vview->view,"Continue",MESG_SHOWTIME);
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
	video->inkey = 0;
	video_post_input(video);
	return 1; /* return 0 to stop */
}
/*----------------------------------------------------------------------------*/
gint on_key_press(GtkWidget *widget, GdkEventKey *kevent, gpointer data)
{
	if(kevent->type == GDK_KEY_PRESS)
	{
		my1video_t *video = (my1video_t*) data;
		/*
		 * g_message("%d, %c", kevent->keyval, kevent->keyval);
		 */
		video->inkey = (int) kevent->keyval;
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1video_t cMain;
	my1video_capture_t cCapture;
	my1video_display_t cDisplay;
	my1image_filter_t grayfilter;
	my1image_filter_t edgefilter;
	my1image_filter_t morefilter;
	my1image_filter_t *pfilter = 0x0;
	int loop, stop = 0;
	char *pfilename = 0x0, *pdevice = 0x0;
	guint timer;

	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");

	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (!strcmp(argv[loop],"--live"))
		{
			if (pdevice)
			{
				printf("Multiple source? (%s&%s)\n",pdevice,argv[loop]);
				stop++;
			}
			else if (loop<argc-1) /* still with param! */
			{
				/* on linux this should be /dev/video0 or something... */
				pdevice = argv[++loop];
			}
			else
			{
				printf("No param for '--live'?\n");
				stop++;
			}
		}
		else
		{
			if (pfilename)
			{
				printf("Multiple source? (%s&%s)\n",pfilename,argv[loop]);
				stop++;
			}
			else
			{
				pfilename = argv[loop];
			}
		}
	}
	if (stop) return -stop;
	if (!pfilename&&!pdevice)
	{
		printf("No video source requested!\n");
		exit(-1);
	}

	/* initialize gui */
	gtk_init(&argc,&argv);

	/* initialize main structures */
	video_init(&cMain);
	capture_init(&cCapture,&cMain);
	display_init(&cDisplay,&cMain);
	cDisplay.view.draw_more = &video_draw_index;
	cDisplay.view.draw_more_data = (void*)&cMain;
	filter_init(&grayfilter,filter_gray,0x0);
	filter_init(&edgefilter,filter_laplace_1,0x0);
	filter_init(&morefilter,0x0,0x0);
	pfilter = filter_insert(pfilter,&grayfilter);
	pfilter = filter_insert(pfilter,&edgefilter);
	pfilter = filter_insert(pfilter,&morefilter);
	video_filter_init(&cMain,pfilter);

	/* setup devices */
	if (pfilename)
		capture_file(&cCapture,pfilename);
	else if (pdevice)
		capture_live(&cCapture,pdevice);

	display_make(&cDisplay);
	display_draw(&cDisplay);
	display_name(&cDisplay, "MY1 Video Test",0x0);

	printf("Starting main capture loop.\n\n%s",showkeys);

	/* setup signals/timer */
	g_signal_connect(G_OBJECT(cDisplay.view.window),"key_press_event",
		G_CALLBACK(on_key_press),(gpointer)&cMain);
	timer = g_timeout_add (FRAME_NEXT_MS,on_timer_callback,(gpointer)&cMain);

	/* main loop */
	gtk_main();
	printf("\n");

	/* clean up */
	g_source_remove (timer);
	filter_free(&morefilter);
	filter_free(&edgefilter);
	filter_free(&grayfilter);
	display_free(&cDisplay);
	capture_free(&cCapture);
	video_free(&cMain);

	return 0;
}
/*----------------------------------------------------------------------------*/
