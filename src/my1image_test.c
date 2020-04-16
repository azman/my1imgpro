/*----------------------------------------------------------------------------*/
#include "my1image_main.h"
#include "my1image_data.h"
#include "my1image_file.h"
#include "my1image_file_png.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#define MY1APP_NAME "my1image_test"
#define MY1APP_INFO "MY1Image Test Program"
#ifndef MY1APP_VERS
#define MY1APP_VERS "build"
#endif
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_data_t what;
	my1imain_t data;
	my1iwork_t work;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* add png support */
	image_format_insert(&ipng);
	/* work it! */
	iwork_make(&work,&what);
	work.init.task = image_data_init;
	work.free.task = image_data_free;
	work.args.task = image_data_args;
	work.prep.task = image_data_prep;
	work.proc.task = image_data_exec;
	work.show.task = image_data_show;
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
