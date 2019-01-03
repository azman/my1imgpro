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
	"\t<SPACE> - pause/play video\n"
	"\tf       - next frame\n"
	"\tb       - previous frame\n"
	"\tl       - toggle looping\n"
	"\tz       - toggle filter\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
/** externally-defined filter functions */
extern my1image_t* filter_gray(my1image_t* img, my1image_t* res,
	my1vpass_t* filter);
extern my1image_t* filter_laplace(my1image_t* img, my1image_t* res,
	my1vpass_t* filter);
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
	my1vmain_t vmain;
	my1vpass_t grayfilter;
	my1vpass_t edgefilter;
	int loop, stop = 0;
	char *pfilename = 0x0, *pdevice = 0x0;
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
	/* check video source */
	if (!pfilename&&!pdevice)
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
	filter_init(&grayfilter,filter_gray,0x0);
	filter_init(&edgefilter,filter_laplace,0x0);
	video_filter_insert(&vmain.video,&grayfilter);
	video_filter_insert(&vmain.video,&edgefilter);
	/* setup devices */
	if (pfilename)
		capture_file(&vmain.vgrab,pfilename);
	else if (pdevice)
		capture_live(&vmain.vgrab,pdevice);
	/* setup display */
	display_make(&vmain.vview);
	display_draw(&vmain.vview);
	display_name(&vmain.vview, "MY1 Video Test",0x0);
	/* tell them */
	printf("Starting main capture loop.\n\n%s",showkeys);
	/* setup display/capture cycle */
	display_loop(&vmain.vview,DEFAULT_LOOP_TIME);
	/* main loop */
	gtk_main();
	/* clean up */
	filter_free(&edgefilter);
	filter_free(&grayfilter);
	video_main_free(&vmain);
	/* we are done */
	printf("\n");
	return 0;
}
/*----------------------------------------------------------------------------*/
