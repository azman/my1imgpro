/*----------------------------------------------------------------------------*/
#include "my1video.h"
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
	"\t]       - next frame\n"
	"\t[       - previous frame\n"
	"\tl       - toggle looping\n"
	"\tz       - toggle filter\n"
	"\tq@<ESC> - quit program\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, type = VIDEO_SOURCE_NONE;
	char *psource = 0x0;
	my1vmain_t vmain;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (argv[loop][0]!='-')
		{
			psource = argv[loop];
			type = VIDEO_SOURCE_FILE;
			break;
		}
		else if (!strcmp(argv[loop],"--live"))
		{
			/* on linux this should be /dev/video0 or something... */
			psource = argv[++loop];
			type = VIDEO_SOURCE_LIVE;
			break;
		}
		else printf("-- Unknown param '%s'!\n",argv[loop]);
	}
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
	/* setup filters */
	for (++loop;loop<argc;loop++)
		video_main_pass_load(&vmain,argv[loop]);
	/* setup capture */
	video_main_capture(&vmain,psource,type);
	/* setup display */
	video_main_display(&vmain,MY1APP_INFO);
	/* prepare menu & events */
	video_main_prepare(&vmain);
	/* tell them */
	printf("Starting main capture loop.\n\n%s",showkeys);
	/* setup display/capture cycle */
	video_main_loop(&vmain,VGRAB_DELAY);
	/* main loop */
	gtk_main();
	/* clean up */
	video_main_free(&vmain);
	/* we are done */
	printf("\n");
	return 0;
}
/*----------------------------------------------------------------------------*/
