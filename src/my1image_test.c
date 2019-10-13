/*----------------------------------------------------------------------------*/
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
	int test;
	char *pname = 0x0;
	my1image_data_t data;
	my1image_t that;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* add png support */
	image_format_insert(&ipng);
	/* initialize that */
	image_init(&that);
	/* check program argument */
	if(argc>1)
	{
		pname = argv[1];
		if((test=image_load(&that,pname))<0)
		{
			printf("Error loading file '%s'!(%x)\n",pname,(unsigned)test);
			return test;
		}
	}
	else
	{
		/* create blank image at default size */
		image_make(&that,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&that,BLACK);
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* initialize image_test */
	image_data_init(&data);
	/* setup auto-quit on close */
	data.view.goquit = 1;
	/* assign image */
	image_data_make(&data,&that);
	image_data_draw(&data);
	image_free(&that);
	/* allow histogram */
	image_hist_make(&data.hist);
	/* event handlers */
	image_data_events(&data);
	/* menu stuff */
	image_data_domenu(&data);
	/* main loop */
	gtk_main();
	/* cleanup image_test */
	image_data_free(&data);
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
