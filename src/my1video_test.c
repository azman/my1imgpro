/*----------------------------------------------------------------------------*/
#include "my1video_main.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1video_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "MY1Video Test Program"
#endif
/*----------------------------------------------------------------------------*/
static char showkeys[] =
{
	"\t---------------------------\n"
	"\tDefined Keys:\n"
	"\t---------------------------\n"
	"\tc       - play video\n"
	"\ts       - stop video\n"
	"\t<SPACE> - pause/play video\n"
	"\tf       - next frame\n"
	"\tb       - previous frame\n"
	"\tl       - toggle looping\n"
	"\tz       - toggle filter\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
void video_draw_index(void* data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	my1video_t* video = (my1video_t*)view->draw_more_data;
	my1vgrab_t* vgrab = (my1video_capture_t*)video->capture;
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
int main(int argc, char* argv[])
{
	int loop, stop = 0, type = VIDEO_SOURCE_NONE;
	char *psource = 0x0;
	my1vmain_t vmain;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (!strcmp(argv[loop],"--live"))
		{
			if (psource)
			{
				printf("Multiple source? (%s&%s)\n",psource,argv[loop]);
				stop++;
			}
			else if (loop<argc-1) /* still with param! */
			{
				/* on linux this should be /dev/video0 or something... */
				psource = argv[++loop];
				type = VIDEO_SOURCE_LIVE;
			}
			else
			{
				printf("No param for '--live'?\n");
				stop++;
			}
		}
		else
		{
			if (psource)
			{
				printf("Multiple source? (%s&%s)\n",psource,argv[loop]);
				stop++;
			}
			else
			{
				psource = argv[loop];
				type = VIDEO_SOURCE_FILE;
			}
		}
	}
	if (stop) return -stop;
	/* check video source */
	if (!psource)
	{
		printf("No video source requested!\n");
		exit(-1);
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* initialize */
	video_main_init(&vmain);
	vmain.vview.view.draw_more = &video_draw_index;
	vmain.vview.view.draw_more_data = (void*)&vmain;
	/* setup filters */
	video_main_pass_load(&vmain,IFNAME_GRAYSCALE);
	video_main_pass_load(&vmain,IFNAME_LAPLACE);
	/* setup capture */
	video_main_capture(&vmain,psource,type);
	/* setup display */
	video_main_display(&vmain,MY1APP_PROGINFO);
	/* tell them */
	printf("Starting main capture loop.\n\n%s",showkeys);
	/* setup display/capture cycle */
	video_main_loop(&vmain,DEFAULT_LOOP_TIME);
	/* main loop */
	gtk_main();
	/* clean up */
	video_main_free(&vmain);
	/* we are done */
	printf("\n");
	return 0;
}
/*----------------------------------------------------------------------------*/
