/*----------------------------------------------------------------------------*/
#include "my1video_main.h"
#include "my1image_main.h"
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
	my1vmain_t what;
	my1imain_t data;
	my1iwork_t work;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
	printf("  => by azman@my1matrix.org\n\n%s\n",showkeys);
	/* work it! */
	iwork_make(&work,&what);
	work.init.task = video_main_init;
	work.free.task = video_main_free;
	work.args.task = video_main_args;
	work.prep.task = video_main_prep;
	work.proc.task = video_main_exec;
	work.show.task = video_main_show;
	imain_init(&data,&work);
	imain_args(&data,argc,argv);
	imain_prep(&data);
	imain_proc(&data);
	if (!(data.flag&IFLAG_ERROR)) gtk_init(&argc,&argv);
	imain_show(&data);
	if (!(data.flag&IFLAG_ERROR)) gtk_main();
	imain_free(&data);
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
