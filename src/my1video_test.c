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
/*----------------------------------------------------------------------------*/
gboolean on_timer_callback(gpointer data)
{
	my1video_t* video = (my1video_t*) data;
	guint keyval = (guint)video->inkey;
	capture_grab((my1video_capture_t*)video->capture);
	if(video->flags&VIDEO_FLAG_NEW_FRAME)
	{
		video_filter(video);
		if (video->count>=0)
			printf("Video frame index: %d/%d\n", video->index, video->count);
		display_view((my1video_display_t*)video->display);
	}
	if(keyval == GDK_KEY_Escape||
		keyval == GDK_KEY_q)
	{
		printf("User abort!\n");
		gtk_main_quit();
	}
	else if(keyval == GDK_KEY_c)
	{
		printf("Play.\n");
		video_play(video);
	}
	else if(keyval == GDK_KEY_s)
	{
		printf("Stop.\n");
		video_stop(video);
	}
	else if(keyval == GDK_KEY_space)
	{
		if (video->flags&VIDEO_FLAG_IS_PAUSED)
		{
			video->flags &= ~VIDEO_FLAG_IS_PAUSED;
			printf("Continue.\n");
		}
		else
		{
			video->flags |= VIDEO_FLAG_IS_PAUSED;
			printf("Paused.\n");
		}
	}
	else if(keyval == GDK_KEY_f)
	{
		printf("Next.\n");
		video_next_frame(video);
	}
	else if(keyval == GDK_KEY_b)
	{
		printf("Previous.\n");
		printf("Not implemented... yet!\n");
		/**video_prev_frame(video);*/
	}
	else if(keyval == GDK_KEY_l)
	{
		if (video->flags&VIDEO_FLAG_LOOP)
		{
			video->flags &= ~VIDEO_FLAG_LOOP;
			printf("Looping OFF.\n");
		}
		else
		{
			video->flags |= VIDEO_FLAG_LOOP;
			printf("Looping ON.\n");
		}
	}
	else if(keyval == GDK_KEY_h)
	{
		printf("%s",showkeys);
	}
	else if(keyval == GDK_KEY_z)
	{
		if (video->flags&VIDEO_FLAG_NO_FILTER)
		{
			video->flags &= ~VIDEO_FLAG_NO_FILTER;
			printf("Filter ON.\n");
		}
		else
		{
			video->flags |= VIDEO_FLAG_NO_FILTER;
			printf("Filter OFF.\n");
		}
	}
	video->inkey = 0;
	video_post_frame(video);
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
gboolean on_draw_expose(GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data)
{
	my1video_display_t* object = (my1video_display_t*) user_data;
	display_draw(object);
	return TRUE;
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
	cMain.flags |= VIDEO_FLAG_NO_FILTER;
	capture_init(&cCapture,&cMain);
	display_init(&cDisplay,&cMain);
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
	display_name(&cDisplay, "MY1 Video Test",0x0);

	printf("Press 'h' for hotkeys.\n");
	printf("Starting main capture loop.\n");

	/* setup signals/timer */
	gtk_signal_connect(GTK_OBJECT(cDisplay.dodraw),"expose-event",
		GTK_SIGNAL_FUNC(on_draw_expose),(gpointer)&cDisplay);
	g_signal_connect(G_OBJECT(cDisplay.window),"key_press_event",
		G_CALLBACK(on_key_press),(gpointer)&cMain);
	g_signal_connect(G_OBJECT(cDisplay.window),"destroy",
		G_CALLBACK(gtk_main_quit),0x0);
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
