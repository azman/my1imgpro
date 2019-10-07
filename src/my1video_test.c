/*----------------------------------------------------------------------------*/
#include "my1video_main.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
/*----------------------------------------------------------------------------*/
#define MY1APP_NAME "my1video_test"
#define MY1APP_INFO "MY1Video Test Program"
#ifndef MY1APP_VERS
#define MY1APP_VERS "build"
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
	"\tq       - pause/play video\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, stop = 0, type = VIDEO_SOURCE_NONE;
	char *psource = 0x0;
	my1vmain_t vmain;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
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
	vmain.vview.view.draw_more = &video_main_draw_index;
	vmain.vview.view.draw_more_data = (void*)&vmain;
	/* setup filters */
	video_main_pass_load(&vmain,IFNAME_GRAYSCALE);
	video_main_pass_load(&vmain,IFNAME_SOBEL);
	video_main_pass_load(&vmain,IFNAME_INVERT);
	video_main_pass_load(&vmain,IFNAME_THRESHOLD);
	video_main_pass_load(&vmain,IFNAME_INVERT);
	/* setup capture */
	video_main_capture(&vmain,psource,type);
	/* setup display */
	video_main_display(&vmain,MY1APP_INFO);
	/* prepare menu & events */
	video_main_prepare(&vmain);
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
